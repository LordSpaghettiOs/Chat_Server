#include "client.h"

int client(char *server_ip, char *port)
{
  int flags;
  int sock, connected, try_number;

  struct sockaddr_in server_address;

  char message_sent[BUFFER_SIZE];
  char message_recv[BUFFER_SIZE];
  char command[BUFFER_SIZE];


  //create socket
  sock = socket(AF_INET, SOCK_STREAM, 0);


  /** Ignore SigPipe Errors
   *
   *  these occur if the user disconnects (CTRL-C) from the server
   *  before choosing a nickname.
   *
   *  This keeps the server from crashing
   */
  signal(SIGPIPE, SIG_IGN);


  //set stdin to be non-blocking
  flags = fcntl(STDIN_FD, F_GETFL, 0);
  fcntl(STDIN_FD, F_SETFL, flags | O_NONBLOCK);


  //initialize the socket addresses
  memset(&server_address, '\0', sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(server_ip);
  server_address.sin_port = htons(atoi(port));

  printf("Attempting to connect to server %s on port %s\n", server_ip, port);

  //client  connect to server on port
  connected = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));

  if (connected != CONNECTION_SUCCESS)
  {
    printf("Unable to connect to server.  Trying again in %d seconds.\n\n", CONNECTION_RETRIES);
    try_number = 1;
    sleep(SLEEP_TIME_IN_SEC);

    while (try_number <= CONNECTION_RETRIES)
    {
      printf("Connecting to server. This is retry %d of %d\n", try_number, CONNECTION_RETRIES);
      connected = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));

      if (connected == CONNECTION_SUCCESS)
        break;

      printf("Unable to connect to server.  Trying again in %d seconds.\n\n", CONNECTION_RETRIES);
      try_number++;

      sleep(SLEEP_TIME_IN_SEC);
    }

    if (try_number > CONNECTION_RETRIES)
    {
      printf("Unable to connect to server after %d tries.  I'm giving up.\n", CONNECTION_RETRIES);
      exit(EXIT_FAILURE);
    }
  }

  printf("Connected to server\n\n");

  //send to sever and receive from server
  while (true)
  {
    memset(message_sent, '\0', sizeof(message_sent));
    memset(message_recv, '\0', sizeof(message_recv));
    memset(command, '\0', sizeof(command));

    while (anything_there(sock, message_recv) > 0)
    {
      printf("%s", message_recv);
      // this solves the odd duplication issues with /who and the banner
      // a cleaner solution would be to assure that \o was appended to all input
      memset(message_recv, '\0', BUFFER_SIZE);
    }


    fgets(command, BUFFER_SIZE, stdin);
    strcpy(message_sent, command);

    command[strcspn(command, "\n")] = 0;
    command[strlen(command)] = '\0';

    send(sock, message_sent, strlen(message_sent), 0);

    if (strcasecmp(command, "/quit") == 0)
      break;

  }

  return 0;
}


ssize_t anything_there(int sock, char *message_recv)
{
  int status;
  struct timeval timeout;

  ssize_t retval;
  fd_set read_mask;

  timeout.tv_sec = TIMEOUT_SECONDS;
  timeout.tv_usec = TIMEOUT_uSECONDS;
  FD_ZERO(&read_mask);
  FD_SET(sock, &read_mask);

  status = select(sock + 1, &read_mask, 0, 0, &timeout);

  if (status == 0)
    return RECV_TIMEOUT;

  if (status == -1)
    return RECV_ERROR;

  retval = recv(sock, message_recv, BUFFER_SIZE, 0);

  return retval;
}

