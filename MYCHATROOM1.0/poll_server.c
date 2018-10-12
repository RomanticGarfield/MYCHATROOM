#define _GNU_SOURCE 2
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <assert.h>
#include <errno.h>

#include <fcntl.h>
#include <poll.h>

#define USER_LIMIT 5  /*最大用户数量 */
#define BUFFER_SIZE 64 /*读缓冲区的大小 */

typedef struct client_data
{
	struct sockaddr_in address;
	char* write_buf;
	char buf[BUFFER_SIZE];
}client_data;

void usage()
{
	printf("usage:/poll_server port");
}

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

int main(int argc, char* argv[])
{
	if(argc != 3){
		usage();
		return 1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("socket");
		return 2;
	}
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(atoi(argv[2]));
	local.sin_addr.s_addr = inet_addr(argv[1]);

	int ret = bind(sockfd, (struct sockaddr*)&local, sizeof(local));
	assert(ret >= 0);

	ret = listen(sockfd, 5);
	assert(ret != -1);

	client_data users[65535];

	struct pollfd fds[USER_LIMIT+1];
	int user_counter = 0;
	for(int i = 1; i <= USER_LIMIT; ++i)
	{
		fds[i].fd = -1;
		fds[i].events = 0;
	}
	fds[0].fd = sockfd;
	fds[0].events = POLLIN | POLLERR;
	fds[0].revents = 0; /*由内核填充*/


	while(1){
		ret  = poll(fds, user_counter+1, -1);
		if(ret < 0)
		{
			printf("poll failure\n");
			break;
		}
		for(int i = 0; i < user_counter+1; ++i)
		{
			if((fds[i].fd == sockfd) && (fds[i].revents & POLLIN))
			{
				struct sockaddr_in client;
				socklen_t len = sizeof(client);
				int connfd = accept(sockfd, (struct sockaddr*)&client, &len);
				if(connfd < 0)
				{
					perror("accept");
					continue;
				}

				if( user_counter >= USER_LIMIT)
				{
					const char* info = "the number of user is out of limit\n";
					printf("%s", info);
					send(connfd, info, strlen(info), 0);
					close(sockfd);
					continue;
				}

				user_counter++;
				users[connfd].address = client;
				setnonblocking(connfd);

				fds[user_counter].fd = connfd;
				fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
				fds[user_counter].revents = 0;
				printf("come a new user, now there are %d users..\n", user_counter );
			}
			else if(fds[i].revents & POLLERR)
			{
				printf("get an error from %d\n",fds[i].fd);
				continue;
			}
			else if(fds[i].revents & POLLRDHUP)
			{
				/*客户端关闭连接，则服务器也关闭连接，并将用户总数减1 */
				users[fds[i].fd] = users[fds[user_counter].fd];
				close(fds[i].fd);
				fds[i] = fds[user_counter];
				i--;
				user_counter--;
				printf("a client left\n" );
			}
			else if(fds[i].revents & POLLIN)
			{	/*监听已链接客户端的写操作 */
				int connfd = fds[i].fd;
				memset(users[connfd].buf, '\0', BUFFER_SIZE);
				ret = recv(connfd, users[connfd].buf, BUFFER_SIZE-1, 0);
				if(ret < 0)
				{
					/*读出错则关闭连接*/
					if(errno != EAGAIN)
					{
						close(connfd);
						users[fds[i].fd] = users[fds[user_counter].fd];
						fds[i] = fds[user_counter];
						i--;
						user_counter--;
					}
				}
				else if(ret > 0)
				{
					printf("get %d bytes of client data %s from %d\n",ret, users[connfd].buf, connfd);
					for(int j = 1; j <= user_counter; ++j)
					{
							if(fds[j].fd == connfd)
								continue;
						fds[j].events |= ~POLLIN;
						fds[j].events |= POLLOUT;
						users[fds[j].fd].write_buf = users[connfd].buf;
					}
				}
			}

			else if(fds[i].revents & POLLOUT)
			{
				int connfd = fds[i].fd;
				if(!users[connfd].write_buf)
				{
					continue;
				}
				ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
				users[connfd].write_buf = NULL;

				/*写完数据后要重新注册fds[i]上的可读事件*/
				fds[i].events |= ~POLLOUT;
				fds[i].events |= POLLIN;
			}
		}
		

	}

	close(sockfd);
	return 0;
}
