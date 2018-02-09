#ifndef CHAT_SERVER_SERVER_H
#define CHAT_SERVER_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include <getopt.h>
#include <netdb.h>


#define MAX_CLIENTS    25
#define MAX_NAME_SIZE  16
#define BUFFER_SIZE    256  //was 1024


typedef struct
{
  struct sockaddr_in addr;
  int socket;
  int whisper_reply_socket;
  char name[MAX_NAME_SIZE];
} client_t;



// Function Prototypes
int client_count();
int server(char *server_ip, char *port);

bool is_valid_name(char *name);

void *handle_client(void *arg);

void queue_add(client_t *client);
void queue_delete(int socket);
void send_server_whisper(const char *msg, int sock);
void send_message(char *message, int socket);
void global_message(char *s);
void strip_newline(char *string);
void display_banner(char *msg_out, client_t *client);
void set_client_name(client_t *client, char *name);
void client_disconnect(char *msg_out, client_t *client);
void announce_new_client(char *msg_out, client_t *client);
void send_logon_message(char *msg_out, client_t *client);

void command_who(char *msg_out, client_t *client);
void command_whisper(char *msg_out, client_t *client);
void command_whisper_helper(char *whisper, char *message_target, int sender);
void command_reply(char *msg_out, client_t *client);
void command_reply_helper(char *whisper, int message_target, int sender);
void command_rename(char *msg_out, client_t *client, char *name);
void command_help(char *msg_out, client_t *client);
void command_error(client_t *client);

#endif
