#include "server.h"

client_t *clients[MAX_CLIENTS];


int server(char *server_ip, char *port)
{

  int yes = 1;
  int server_socket = 0;
  int client_socket = 0;

  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;

  pthread_t thread_id;

  //int client_id = 9001;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip);
  server_addr.sin_port = htons(atoi(port));

  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
  {
    perror("Bind");
    exit(-2);
  }

  if (listen(server_socket, MAX_CLIENTS) < 0)
  {
    perror("Listen");
    exit(-1);
  }

  printf(" Server Ready...\n");
  printf("    IP Address: %s\n", server_ip);
  printf("   Port Number: %s\n", port);
  printf(" Process ID Is: %d\n\n", getpid());

  /** ignore sigpipe errors
   *  occurs when client drops before a socket write can happen
   *  e.g. CTRL-C when asked for a nickname
   */
  signal(SIGPIPE, SIG_IGN);

  while (true)
  {
    socklen_t client_length = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_length);

    client_t *client = (client_t *) malloc(sizeof(client_t));
    client->addr = client_addr;
    client->socket = client_socket;
    client->whisper_reply_socket = -1;
    sprintf(client->name, "Unnamed");

    queue_add(client);
    pthread_create(&thread_id, NULL, &handle_client, (void *) client);
  }
}


void *handle_client(void *arg)
{
  client_t *client = (client_t *) arg;

  char msg_out[BUFFER_SIZE];
  char msg_in[BUFFER_SIZE];
  char name[MAX_NAME_SIZE];

  display_banner(msg_out, client);

  printf("\nClient connected: %s\n", inet_ntoa(client->addr.sin_addr));

  set_client_name(client, name);
  announce_new_client(msg_out, client);
  send_logon_message(msg_out, client);

  //process_client_data(msg_in, msg_out, client, name);

  char *command;
  ssize_t bytes_recvd;

  while ((bytes_recvd = recv(client->socket, msg_in, sizeof(msg_in) - 1, 0)) > 0)
  {
    msg_in[bytes_recvd] = '\0';
    strip_newline(msg_in);

    // If there's no data in the buffer, the user pressed Enter...
    if (!strlen(msg_in))
      continue;

    // Chat Commands
    if (msg_in[0] == '/')
    {
      command = strtok(msg_in, " ");

      if (!strcasecmp(command, "/quit"))
        break;

      else if (!strcasecmp(command, "/rename"))
        command_rename(msg_out, client, name);

      else if (!strcasecmp(command, "/w"))
        command_whisper(msg_out, client);

      else if (!strcasecmp(command, "/r"))
        command_reply(msg_out, client);

      else if (!strcasecmp(command, "/who"))
        command_who(msg_out, client);

      else if (!strcasecmp(command, "/help"))
        command_help(msg_out, client);

      else
        command_error(client);
    }
    else
    {
      sprintf(msg_out, "[%s] %s\n", client->name, msg_in);
      send_message(msg_out, client->socket);
    }
  }

  client_disconnect(msg_out, client);
}


void send_logon_message(char *msg_out, client_t *client)
{
  sprintf(msg_out, "[SERVER] To see a list of server commands, type: /help\n\n");
  send_server_whisper(msg_out, client->socket);
}


void announce_new_client(char *msg_out, client_t *client)
{
  sprintf(msg_out, "[SERVER] %s has joined the server.\n", client->name);
  printf("   Client at address %s is known as %s.\n", inet_ntoa(client->addr.sin_addr), client->name);
  global_message(msg_out);
}


void client_disconnect(char *msg_out, client_t *client)
{
  close(client->socket);
  sprintf(msg_out, "[SERVER] %s has left the server.\n", client->name);
  printf("\nClient disconnected: %s (Name: %s)\n", inet_ntoa(client->addr.sin_addr), client->name);

  global_message(msg_out);
  queue_delete(client->socket);
  free(client);
  pthread_detach(pthread_self());
}


int client_count()
{
  int i;
  int count = 0;

  for (i = 0; i < MAX_CLIENTS; ++i)
  {
    if (clients[i])
      count++;
  }

  return count;
}


void set_client_name(client_t *client, char *name)
{
  char *enter_name = "Enter nickname: ";

  do
  {
    send(client->socket, enter_name, strlen(enter_name), 0);
    recv(client->socket, name, MAX_NAME_SIZE, 0);
    strip_newline(name);
  } while (!is_valid_name(name));

  strcpy(client->name, name);
}


void display_banner(char *msg_out, client_t *client)
{
  sprintf(msg_out, "******************************\n");
  strcat(msg_out,  "*                            *\n");
  strcat(msg_out,  "* Welcome to the chat server *\n");
  strcat(msg_out,  "*                            *\n");
  strcat(msg_out,  "******************************\n\n\0");

  send_server_whisper(msg_out, client->socket);
}


void command_error(client_t *client)
{
  char *error_unknown_cmd = "Error: unknown command.\n\n";
  send(client->socket, error_unknown_cmd, strlen(error_unknown_cmd), 0);
}


void command_help(char *msg_out, client_t *client)
{
  strcat(msg_out, "Server Commands:\n");
  strcat(msg_out, "  /w       <name> <message>   Send whisper\n");
  strcat(msg_out, "  /r                          Reply to whisper\n");
  strcat(msg_out, "  /who                        Show active clients\n");
  strcat(msg_out, "  /rename  <name>             Change nickname\n");
  strcat(msg_out, "  /help                       Shows this message\n");
  strcat(msg_out, "  /quit                       Quit chatroom\n\n");

  send(client->socket, msg_out, strlen(msg_out), 0);
}


void command_reply(char *msg_out, client_t *client)
{
  char *param;
  char *error_null_msg = "Error: message cannot be NULL.\n\n";
  char *error_no_whisper_msg = "Error: no one to reply to.\n\n";

  if (client->whisper_reply_socket != -1)
  {
    param = strtok(NULL, " ");
    if (param)
    {
      sprintf(msg_out, "[reply][%s]", client->name);
      while (param != NULL)
      {
        strcat(msg_out, " ");
        strcat(msg_out, param);
        param = strtok(NULL, " ");
      }
      strcat(msg_out, "\n");

      command_reply_helper(msg_out, client->whisper_reply_socket, client->socket);
    }
    else
    {
      send(client->socket, error_null_msg, strlen(error_null_msg), 0);
    }
  }
  else
  {
    send(client->socket, error_no_whisper_msg, strlen(error_no_whisper_msg), 0);
  }
}


void command_whisper(char *msg_out, client_t *client)
{
  char *param;
  char *error_null_msg = "Error: message cannot be NULL.\n\n";
  char *error_null_name = "Error: name cannot be NULL.\n\n";

  param = strtok(NULL, " ");
  if (param)
  {
    char *message_target = param;
    param = strtok(NULL, " ");
    if (param)
    {
      sprintf(msg_out, "[whisper][%s]", client->name);
      while (param != NULL)
      {
        strcat(msg_out, " ");
        strcat(msg_out, param);
        param = strtok(NULL, " ");
      }
      strcat(msg_out, "\n");
      command_whisper_helper(msg_out, message_target, client->socket);
    }
    else
    {
      send(client->socket, error_null_msg, strlen(error_null_msg), 0);
    }
  }
  else
  {
    send(client->socket, error_null_name, strlen(error_null_name), 0);
  }
}


void command_rename(char *msg_out, client_t *client, char *name)
{
  char *param, *old_name;
  char *error_invalid_name = "Error: name is invalid or in use. Try again.\n\n";
  char *error_null_name = "Error: name cannot be NULL.\n\n";

  bool valid_name = true;
  param = strtok(NULL, " ");

  if (param)
  {
    do
    {
      if (!valid_name)
      {
        send(client->socket, error_invalid_name, strlen(error_invalid_name), 0);
        recv(client->socket, param, MAX_NAME_SIZE, 0);
      }
      strip_newline(param);
      valid_name = false;
    } while (!is_valid_name(param));


    strcpy(client->name, name);
    old_name = strdup(client->name);

    strcpy(client->name, param);
    sprintf(msg_out, "[SERVER] %s is now %s\n", old_name, client->name);
    printf("   %s is now %s\n", old_name, client->name);
    free(old_name);
    global_message(msg_out);
  }
  else
  {
    send(client->socket, error_null_name, strlen(error_null_name), 0);
  }
}


void send_message(char *message, int socket)
{
  int i;
  for (i = 0; i < MAX_CLIENTS; ++i)
  {
    if (clients[i] && clients[i]->socket != socket)
      send(clients[i]->socket, message, strlen(message), 0);
  }
}


void global_message(char *s)
{
  int i;
  for (i = 0; i < MAX_CLIENTS; ++i)
  {
    if (clients[i])
      send(clients[i]->socket, s, strlen(s), 0);
  }
}


void command_whisper_helper(char *whisper, char *message_target, int sender)
{
  int i;

  for (i = 0; i < MAX_CLIENTS; ++i)
  {
    if (clients[i] && strcasecmp(clients[i]->name, message_target) == 0)
    {
      send(clients[i]->socket, whisper, strlen(whisper), 0);
      clients[i]->whisper_reply_socket = sender;
      return;
    }
  }
}

void command_reply_helper(char *whisper, int message_target, int sender)
{
  int i;

  for (i = 0; i < MAX_CLIENTS; ++i)
  {
    if (clients[i] && clients[i]->socket == message_target)
    {
      send(clients[i]->socket, whisper, strlen(whisper), 0);
      clients[i]->whisper_reply_socket = sender;
      return;
    }
  }
}

void command_who(char *msg_out, client_t *client)
{
  int i;
  //char who[BUFFER_SIZE];

  sprintf(msg_out, "Client Count: %d\n", client_count());
  send(client->socket, msg_out, strlen(msg_out), 0);

  for (i = 0; i < MAX_CLIENTS; ++i)
  {
    if (clients[i])
    {
      sprintf(msg_out, " %d: %s\n", i + 1, clients[i]->name);
      send(client->socket, msg_out, strlen(msg_out), 0);
    }
  }

  send(client->socket, "\n\0", 2, 0);
}


void strip_newline(char *string)
{
  while (*string != '\0')
  {
    if (*string == '\r' || *string == '\n')
      *string = '\0';

    string++;
  }
}




void queue_add(client_t *client)
{
  int i;
  for (i = 0; i < MAX_CLIENTS; ++i)
  {
    if (!clients[i])
    {
      clients[i] = client;
      return;
    }
  }
}


void queue_delete(int socket)
{
  int i;
  for (i = 0; i < MAX_CLIENTS; ++i)
  {
    if (clients[i] && clients[i]->socket == socket)
    {
      clients[i] = NULL;
      return;
    }
  }
}


bool is_valid_name(char *name)
{
  int i, j;
  const char *restricted_names[] = {"SERVER", "ROOT", "ADMIN", "ADMINISTRATOR", "MODERATOR", "MOD", "OWNER"};
  const char restricted_characters[] = {'/', ' ', '[', ']'};
  const int restricted_names_count = sizeof(restricted_names) / sizeof(restricted_names[0]);

  // is name in use
  for (i = 0; i < MAX_CLIENTS; ++i)
    if (clients[i] && strcasecmp(clients[i]->name, name) == 0)
      return false;

  // is name restricted
  for (i = 0; i < restricted_names_count; ++i)
    if (strcasecmp(restricted_names[i], name) == 0)
      return false;

  // is name using restricted characters
  for (i = 0; i < sizeof(restricted_characters); ++i)
  {
    for (j = 0; j < strlen(name); ++j)
    {
      if (restricted_characters[i] == name[j])
      {
        return false;
      }
    }
  }

  return true;
}


void send_server_whisper(const char *msg, int sock)
{
  send(sock, msg, strlen(msg), 0);
}