#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "consts.h"

#define MAX_EVENTS 10

/* fcntl() performs operation on file descriptor */
void setnonblocking(int sockfd)
{
	int status = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

	if (status == -1)
		perror("calling fcntl");
}

int open_listenfd(int port)
{

	char *ip = "127.0.0.1"; // localhost
	int sockfd;
	struct sockaddr_in server_addr;

	// IPv4 UDP socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sockfd < 0)
	{
		perror("[-]socket error");
		exit(EXIT_FAILURE);
	}

	setnonblocking(sockfd);

	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	// Bind socket to a particular port
	int n = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (n < 0)
	{
		perror("[-]bind error");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	return sockfd;
}

const int read_fname_len(char * buff) {

	int r = -1;
	if (atoi(&buff[0]) == 0) {
		r =  atoi(&buff[1]);
	} else {
		char flen[3] = {0};
		strncpy(flen, buff, 2);
		r =  atoi(flen);
	}
	return r;
}

int main()
{
	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock, conn_sock, nfds, epollfd;

	struct sockaddr_in addr;
	int addrlen = sizeof(addr);

	// Socket: Create, bind, and listen on localhost:port
	listen_sock = open_listenfd(8080);

	// Create an epoll instance
	epollfd = epoll_create1(0);

	if (epollfd == -1)
	{
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	// Set to monitor incoming data on socket
	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;

	// Add socket to epoll's interest list
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1)
	{
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	char buffer[MAX_BUFF];

	for (;;)
	{

		// Wait for events
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);

		if (nfds == -1)
		{
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int n = 0; n < nfds; ++n)
		{

			if (events[n].data.fd == listen_sock)
			{

				/*
				listen --> accept approach as described in man pages
				is for SOCK_STREAM (TCP) type sockets.

				For SOCK_DGRAM (UDP) recvfrom is used.
				*/

				// Clear buffer and receive message
				bzero(buffer, MAX_BUFF);
				recvfrom(listen_sock, buffer, MAX_BUFF, 0, (struct sockaddr *)&addr, &addrlen);

				// Read file name
				const int fleni = read_fname_len(buffer);

				if (fleni == -1) {
					perror("packet read error");
					exit(EXIT_FAILURE);
				}

				char fname[FNAME_LEN] = {0};
				strncpy(fname, &buffer[2], fleni);
				
				// Write data to file
				char data[MAX_DATA_LEN] = {0};
				memcpy(data, &buffer[fleni+2], MAX_BUFF - (fleni + 2));

				puts(buffer);

				FILE *fp;

				char path[] = "receive/";
				strcat(path, fname);
				fp  = fopen(path, "a");
				fputs(data, fp);

				fclose(fp);
			}
		}
	}
}