#include "client.h"
#include "server.h"

//TODO: Ensure IP address is valid (both ipv4 and ipv6, check null)
//TODO: Ensure host name is valid (check for null)
//TODO: IPv6 Support

#define ARGS_ERROR   (-1)
#define IPV6_MAX_LENGTH     40


typedef enum
{
  undefined_mode = -1,
  server_mode = 0,
  client_mode
} RUNMODE;


//Function Prototypes
void resolve_host(char *optarg, char address[]);
void help_text(char *argv[]);


int main(int argc, char *argv[])
{
  const int ipv6_max_length = 40;
  const char *short_opt = "ShI:P:H:";

  int opt;
  bool s_flag = false;
  bool h_flag = false;
  bool i_flag = false;
  char address[ipv6_max_length];
  char *port = {NULL};
  char *server_ip = {NULL};

  RUNMODE run_mode = client_mode;

  struct option long_opt[] =
      {
          {"server", no_argument, NULL, 'S'},
          {"port", required_argument, NULL, 'P'},
          {"ip", required_argument, NULL, 'I'},
          {"hostname", required_argument, NULL, 'H'},
          {"help", no_argument, NULL, 'h'},
          {NULL, 0, NULL, 0}
      };


  while ((opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
  {
    switch (opt)
    {
      case -1:       /* no more arguments */
      case 0:        /* long options toggles */
        break;

      case 'S':
        run_mode = server_mode;
        s_flag = true;
        break;

      case 'P':
        port = optarg;
        break;

      case 'I':
        server_ip = optarg;
        i_flag = true;
        break;

      case 'H':
        resolve_host(optarg, address);
        server_ip = address;
        h_flag = true;
        break;

      case 'h':
        help_text(argv);
        return (0);

      case ':':
      case '?':
        printf("Try `%s --help' for more information.\n", argv[0]);
        return (-2);

      default:
        printf("%s: invalid option -- %c\n", argv[0], opt);
        printf("Try `%s --help' for more information.\n", argv[0]);
        return (-2);
    };
  };


  if (h_flag && i_flag)
  {
    printf("Error: Hostname and IP Address options are mutually exclusive.\n\n");
    exit(ARGS_ERROR);
  }


  if (argc < 3 || server_ip == NULL || port == NULL || run_mode == undefined_mode)
  {
    help_text(argv);
    exit(ARGS_ERROR);
  }


  if (s_flag)
    server(server_ip, port);
  else
    client(server_ip, port);

}


void resolve_host(char *optarg, char address[])
{
  int i;

  struct hostent *host;
  struct in_addr **addr_list;

  memset(address, '\0', IPV6_MAX_LENGTH);
  host = gethostbyname(optarg);

  if (!host)
  {
    perror("gethostbyname");
    exit(EXIT_FAILURE);
  }

  addr_list = (struct in_addr **) host->h_addr_list;

  for (i = 0; addr_list[i] != NULL; i++)
    strcpy(address, inet_ntoa(*addr_list[i]));
}


void help_text(char *argv[])
{
  printf("\nUsage: %s [OPTION]\n", argv[0]);
  printf("  -S, --server              Start as server (default is client)\n");
  printf("  -I, --ip                  IP address to use as server, or connect to as client\n");
  printf("  -H, --hostname            Hostname to use as server, or connect to as client\n");
  printf("  -P, --port                Port numberto use as server, or connect to as client\n");
  printf("  -h, --help                Print this help text and exit\n");
  printf("\n");
}