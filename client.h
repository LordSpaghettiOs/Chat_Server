#ifndef CHAT_SERVER_CLIENT_H
#define CHAT_SERVER_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>
#include <getopt.h>
#include <fcntl.h>


#define STDIN_FD            0
#define BUFFER_SIZE         256
#define CONNECTION_RETRIES  3
#define CONNECTION_SUCCESS  0
#define SLEEP_TIME_IN_SEC   3

#define TIMEOUT_SECONDS     0
#define TIMEOUT_uSECONDS    250000

#define RECV_ERROR          (-2)
#define RECV_TIMEOUT        (-1)


int client(char *server_ip, char *port);
ssize_t anything_there(int sock, char *message_recv);

#endif
