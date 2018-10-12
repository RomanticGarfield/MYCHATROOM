// Author :         Cocoon
// Email :          Cocoon0206@163.com
// Last Modified :  2018-08-31 16:27
// Filename :		sercive.h
// Description :    聊天室服务端头文件


#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <sys/epoll.h>

#include "struct.h"



int serve_user_register(send_t *precv); // 注册函数

int serve_user_login(send_t *precv);  // 登录函数

int serve_user_logoff(send_t *precv);  // 下线函数

int serve_user_update(send_t *precv); // 修改基本信息

int serve_friend_show(send_t *precv); // 查看好友列表

int serve_friend_add(send_t *precv);  // 添加好友请求

int serve_friend_find(send_t *precv); // 查找好友

int serve_chat_one(send_t *precv);   // 聊天


// 自定义出错处理函数
void my_err(const char* err_string, int line){
	fprintf(stderr, "line:%d \n", line);
	perror(err_string);
}

// 从套接字上读取一次数据
int recv_data(int conn_fd, send_t *precv){
	int len = recv(conn_fd, precv, sizeof(send_t), 0);
	if(len < 0){
		my_err("recv", __LINE__);
	}
	printf("%d bytes received\n", precv->command_type);
	return len;
}
// 发送数据
int send_data(int conn_fd, send_t* psend){
	int ret = send(conn_fd, psend, sizeof(send_t), 0);
	if(ret < 0){
		my_err("send", __LINE__);
	}

	printf("%u send\n", psend->command_type);
	memset(psend, 0, sizeof(send_t));

	return ret;
}

#endif
