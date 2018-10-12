#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <poll.h>

#define BUFFER_SIZE 64
void usage()
{
	printf("usage(): ./poll-client ip port name");
}

int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		usage();
		return 1;
	}
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("sockfd");
		return 2;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);

	int ret = connect(sockfd, (struct sockaddr*)&server, sizeof(server));
	if(ret < 0){
		printf("connect failed");
		close(sockfd);
		return 3;
	}

	struct pollfd fds[2];
	/* 注册文件描述符0和sockfd上的可读事件 */
	fds[0].fd = 0;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	
	fds[1].fd = sockfd;
	fds[1].events = POLLIN;
	fds[2].revents = 0;

	char read_buf[BUFFER_SIZE];
	int pipefd[2];
	ret = pipe(pipefd);
	if(ret < 0){
		perror("pipe");
		close(sockfd);
		return 4;
	}

	while(1)
	{
		ret = poll(fds, 2, -1);
		if(ret < 0){
			printf("poll failed");
			break;
		}
		if(fds[1].revents & POLLRDHUP)
		{
			printf("server close the connection");
		}
		else if(fds[1].revents & POLLIN)
		{
			memset(read_buf, '\0', BUFFER_SIZE);
			recv(fds[1].fd, read_buf, BUFFER_SIZE-1, 0);
			printf("%s\n", read_buf);
		}
		if(fds[0].revents & POLLIN)
		{
			ret = splice(0, NULL, pipefd[1], NULL, 32768,\
				SPLICE_F_MORE | SPLICE_F_MOVE);

			ret = splice(pipefd[0], NULL, sockfd, NULL, 32768,\
				SPLICE_F_MORE | SPLICE_F_MOVE);
		}
	}
	close(sockfd);
	return 0;
}
