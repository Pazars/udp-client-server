#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "consts.h"

/*Basic program input error handling.*/
int verify_input(int argc, char **argv)
{
  if (argc != 3)
  {
    printf("Usage: %s <port> <fname>\n", argv[0]);
    return -1;
  }

  if (strlen(argv[2]) > FNAME_LEN)
  {
    printf("File name should be less than %d characters.\n", FNAME_LEN);
    printf("'%s' has %d\n", argv[2], (int)strlen(argv[2]));
    return -1;
  }

  return EXIT_SUCCESS;
}

/*Connect UDP socket to localhost:port.*/
int open_clientfd(int port, struct sockaddr_in *addr)
{
  int sockfd;
  char *ip = "127.0.0.1";

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(addr, '\0', sizeof(*addr));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = inet_addr(ip);

  if ((connect(sockfd, (struct sockaddr *)addr, sizeof(*addr)) < 0))
  {
    perror("Client connect");
    close(sockfd);
    return -1;
  }

  return sockfd;
}

/* Print transfer speed in MB/s */
void print_transfer_speed(clock_t t)
{
  double mb = MAX_BUFF * 0.000001;            // Bytes --> MB
  double time = ((double)t) / CLOCKS_PER_SEC; // in seconds
  double speed = mb / time;
  printf("\r");
  fflush(stdout);
  printf("Transfer speed %.2fMB/s", speed);
}

int main(int argc, char **argv)
{

  if (verify_input(argc, argv) == -1)
    exit(EXIT_FAILURE);

  // Get file name
  char fname[FNAME_LEN];
  strncpy(fname, argv[2], FNAME_LEN - 1);
  fname[FNAME_LEN] = '\0';

  // Connect socket
  int port = atoi(argv[1]);
  struct sockaddr_in addr;
  int sockfd = open_clientfd(port, &addr);

  if (sockfd == -1)
    exit(EXIT_FAILURE);

  char buffer[MAX_BUFF] = {0};

  FILE *fp;
  fp = fopen(fname, "r");

  if (fp != NULL)
  {
    // Check file size
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    clock_t t;

    size_t MAX_DATA_SIZE = MAX_BUFF - FNAME_LEN;
    char data[MAX_DATA_SIZE];
    bzero(data, MAX_DATA_SIZE);

    // while (fread(data, sizeof(char), MAX_DATA_SIZE, fp) > 0)
    while (fread(data, 1, MAX_DATA_SIZE, fp) > 0)
    {
      /*
      Packets

      Structure   : [[FILE_NAME_LENGTH] [FILE_NAME] [DATA]                    ]
      Size (bytes): [[2]                [32]        [<= (MAX_BUFFER - 2 - 32)]]

      Always use 3 bytes for the file and data lengths.

      For example:

      If FILE_NAME_LENGTH = 5, format it as 05\0
      If FILE_NAME_LENGTH = 15, format it as 15\0


      */

      // Format file name length
      char fnamelen[3];
      if (strlen(fname) > 9) {
        snprintf(fnamelen, sizeof(fnamelen), "%ld", strlen(fname));
      } else {
        snprintf(fnamelen, sizeof(fnamelen), "0%ld", strlen(fname));
      }

      // Copy data to buffer
      memmove(buffer, fnamelen, sizeof(fnamelen));
      memmove(&buffer[strlen(fnamelen)], fname, strlen(fname));
      memmove(&buffer[strlen(fnamelen)+strlen(fname)], data, strlen(data));

      // Send data and time command execution
      t = clock();
      sendto(sockfd, buffer, MAX_BUFF, 0, (struct sockaddr *)&addr, sizeof(addr));
      t = clock() - t;

      print_transfer_speed(t);

      bzero(data, MAX_DATA_SIZE);
      bzero(buffer, MAX_BUFF);
    }
  }

  fclose(fp);

  close(sockfd);

  return EXIT_SUCCESS;
}
