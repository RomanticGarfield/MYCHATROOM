// Author : Cocoon
// Email : Cocoon0206@163.com
// Last modified : 2018-08-31 08:16
// Filename : service.c
// Description : 服务端主程序

#include "service.h"
#define EPOLL_MAX 1024

int *service(send_t *precv){
	
	switch(precv->command_type)	{
		
		// 注册
		case _USERREG:

			serve_user_register(precv);
			break;
		
		// 登录
		case _USERONL:
			
			serve_user_login(precv);
			break;

		// 下线
		case _USEROFFL:

			serve_user_logoff(precv);
			break;

		// 修改基本信息
		case _USERUPD:
			
			serve_user_update(precv);
			break;

		// 查看好友
		case _FRIENDSHOW:
			
			serve_friend_show(precv);
			break;
		
		// 添加好友
		case _FRIENDADD:

			serve_friend_add(precv);
			break;
		

		// 查找好友
		case _FRIENDFIND:
			
			serve_friend_find(precv);
			break;

		// 私聊
		case _CHATONE:
			
			serve_chat_one(precv);
			break;

		// 群聊
		case _CHATGROUP:

			serve_chat_group(precv);
			break;


		default:
			break;

	}
}


int main(int argc, char *argv[])
{

	if(argc != 2){
		printf("usage: service [port]\n");
		return 1;
	}

	int nums = 0;//当前用户数
	
	// 创建TCP套接字
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		my_err("socket", __LINE__);
	}
	
	// 设置套接字地址可重用
	int optval = 1;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(int) ) < 0){
		my_err("setsockopt", __LINE__);
	}
	
	// 初始化服务器地址
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port  = htons(atoi(argv[1]));
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 绑定
	int ret = bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));;
	if(ret < 0){
		my_err("bind", __LINE__);
	}
	// 监听
	ret = listen(sock_fd, 5);
	if(ret < 0){
		my_err("listen", __LINE__);
	}

	// 注册epoll事件
	struct epoll_event ev, events[EPOLL_MAX];
	int	epfd = epoll_create(EPOLL_MAX);
	ev.data.fd = sock_fd;
	ev.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sock_fd, &ev);
	
	// 定义客户端信息
	struct sockaddr_in cli_addr;
	socklen_t cli_len;
	send_t *precv;

	// 清屏，初始化界面
	system("clear");
	puts("service is working...");

	for(;;){

		//设置永久阻塞 
		int nfds = epoll_wait(epfd, events, 1, -1); 
		if(nfds < 0) break;
		

		int conn_fd;
		for(int i = 0; i < nfds; i++){
			// 处理连接请求
			if(events[i].data.fd == sock_fd){
				// accept客户端连接
				conn_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &cli_len);
				if(conn_fd == -1){
					my_err("accept", __LINE__);
				}
					
				nums++;
				printf("client[IP:%s] is there, All: %d\n", inet_ntoa(cli_addr.sin_addr), nums);

				// 初始化客户端
				ev.data.fd = conn_fd;
				ev.events = EPOLLIN;
				epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);
			}

			// 处理读事件
			else if(events[i].events & EPOLLIN){
				int fd;
				// 开始读数据
				ret = recv_data(events[i].data.fd, precv);
				
				// 判断客户端发送退出请求
				if(ret <= 0 || precv->command_type == _USEROFFL){
					printf("\nclient: %d disconnect\n", events[i].data.fd);
					precv->conn_fd = events[i].data.fd;
					fd = events[i].data.fd;
					serve_user_logoff(precv);
					
					// 从epfd中删除fd
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
					close(fd);

					continue;
				}

				precv->conn_fd = events[i].data.fd;
				fd = events[i].data.fd;
				

				pthread_t id;
				void *flag;
				//创建子线程去处理客户事件
				pthread_create(&id, NULL, (void*)service, precv);
				// 父进程等待
				pthread_join(id, &flag);
			}
		}
	}

	close(epfd);
	
	// 彻底关闭服务
	shutdown(sock_fd, SHUT_RD);
	return 0;
}
