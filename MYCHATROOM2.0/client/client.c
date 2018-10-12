//  Author :          Cooon
//  Email :           Cocoon0206@163.com
//  Last modified :   2018-09-01 08:54
//  Filename :        client.c
//  Description :	  客户端主程序

#include "client.h"

//登录界面
int login_ui(int conn_fd){

	char c = 0;
	int result_login = 0;
	int times_false = 0;
	send_t send_buf,recv_buf;
	send_t *psend = &send_buf;
	send_t *precv = &recv_buf;

	do{	
    	system("clear");
    	printlogo();
    	printf("\033[31m<system> \033[0m:\033[33m welcome \033[0m, log in please\n");
    	
    	//用户名输入
    	printf("username:"); 
    	get_username(32, psend->userinfo.username);	
    	//密码输入
    	printf("password:");
    	get_userpasswd(32, psend->userinfo.userpasswd);
    	//请求登录
    	psend->command_type = _USERONL;
    	psend->conn_fd = conn_fd;
    	send_data(psend->conn_fd, psend);
    
    	recv_data(conn_fd, precv);//获取服务器响应
    
    	if(precv->userstatus == VALID_USERINFO){
    		//登录信息正确，登录成功
    		result_login = 1;
    		puts("Sign in sucess...\n");
    	}
    	else if(precv->userstatus == INVALID_USERINFO){
    		//信息不正确，登录失败
    		if(precv->input_check == _FALSENAME){
    			puts("name not exist.");
    		}
    		else if(precv->input_check == _FALSEPASSWD){
    			puts("wrong password.");
    		}
    		else if(precv->input_check == _ONLINE){
    			puts("user already signed in.");
    		}
			if(times_false++ == 3){//错误超过三次
				puts("the times of false input is out of permission");
				puts("the program will exit 1s later");
				sleep(1);
				exit(0);
			}else{
				puts("you have %d times chances left", 3-times_false);
			}

    	}
	}while(login_result);
	sleep(1);
}

//注册界面
int register_ui(int conn_fd){

	send_t send_buf, recv_buf;
	send_t *psend = &send_buf;
	send_t *precv = &recv_buf;

	userinfo_t info;
	userinfo_t *data = &info;
	
	puts("<system>welcome register:\n");
	puts("\033[31m<system>\033[0m:complete your information first");

	//用户名
	printf("username:");
	get_username(32, psend->userinfo.username);
	//密码
	printf("password:");
	get_r_userpasswd(32, psend->userpasswd);
	//性别
	printf("sex:");
	get_usersex(32, psend->usersex);
	//请求注册
	psend->command_type = _USERADD;
	psend->conn_fd = conn_fd;
	send_data(psend->conn_fd, precv);

	recv_data(conn_fd, precv);//获取服务器响应
	
	if(precv->userstatus == INVALID_USERINFO){
		//注册失败
		puts("register failed");
		return 0;
	}
	else if(precv->user_status == VALID_USERINFO){
		//注册成功
		puts(“register success”);
		puts("logining...");
	}
	sleep(1);
}


//初始化界面
void init_ui(int conn_fd){
	char choice;
	
	//
	system("clear");
	puts("\033[33m welcome ,please choose:\033[0m");
	puts("Sign in");
	puts("Register");
	puts("Exit");
	puts("");

	scanf("%c",&choice);
	  
	switch(choice){
		case 'S':
		case 's':
			login_ui(conn_fd);
			break;

		case 'r':
		case 'R':
			register_ui(conn_fd);
			break;

		case 'e':
		case 'E':
			exit(0);
			break;
	}
	puts("\033[33m.-.-.-.-.-.-.-.-.-.-.-.-\033[0m");
	
}

//登录后用户界面
void userinfo_ui(int conn_fd ){
	char choice;
	system("clear");
	do{
		puts("\033[33mwhat do you what?\033[0m");
		puts("[F]riend list");      // 好友列表
		puts("[G]roup list");       // 群聊列表
		puts("[C]heck message box");// 消息
		puts("[Fi]TP");
		puts("[R]eturn main menu"); 
		puts("[E]xit");

		puts("\033[33m.-.-.-.-.-.-.-.-.-.-.-.-.-.-\033[0m");
		
		choice = getchar();
		switch(choice){
			case 'f':
			case 'F':
				//show the group list
				friend_chat_show(conn_fd);
				break;
			case 'g':
			case 'G':
				group_chat_show(conn_fd);
				break;
			case 'c':
			case 'C':
				message_init(conn_fd);
				break;
			case 'Fi':
				file_ui_init(conn_fd);
				break;
			case 'r':
			case 'R':
				account_ui_init(conn_fd);
				break;
		}
	}while(choice != 'e' && choice != 'E')
	
	account_ui_logoff(conn_fd);
}

int main(int argc, char*argv[]) {
 	
	if( argc != 3){
		printf("Usage: ./client [ip] [port]\n");
		return 1;
	}
	
	//创建TCP套接字
	int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if(conn_fd < 0){
		my_err("socket", __LINE__);
	}
 	
	//初始化服务器地址
	struct sockaddr_in server;
	server.sin_famfily = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
 
 	//向服务器发送连接请求
 	int ret = connect(conn_fd, (struct sockaddr*)&server, sizeof(server));
	if(ret < 0){
		my_err("connet", __LINE__);
		exit(0);
	}
	
	init_ui(conn_fd);// 欢迎界面,登录成功返回

	//创建子线程处理
	pthread_t pid;
	send_t recv_buf;
	send_t *precv = &recv_buf;
	void* flag;

	for(;;){
		ret = recv_data(conn_fd, precv);
		if(ret <= 0){
			printlogo();
			puts("\033[31m404 not found...\033[0m");
			sleep(1);
			account_ui_logoff(conn_fd, precv->userinfo.username);
			break;
		}	
		else{
			pthread_create(&pid, NULL, (void*)&client, precv);
			pthread_join(pid, &flag);
		}
	}
	
	userinfo_ui(conn_fd);//登录后用户界面

	shutdown(conn_fd, SHUT_RD);//退出
}

void client(send_t* precv){
	switch(precv->command_type){
		
		case _FRIENDLIST:
			chat_friend_show(precv);
			break;

		case _FRIENDADD:
			chat_friend_add(precv);
			break;

		case _FRIENDDEL:
			chat_friend_del(precv);
			break;

		case _FRIENDFIND:
			chat_friend_find(precv);
			break;

		case _GROUPLIST:
			chat_group_show(precv);
			break;
		
		case _GROUPADD:
			chat_group_add(precv);
			break;

		case _CHATONE:
			printf("\033[s");
			user_chat_one(precv);
			printf("\033[u");
			fflush(stdout);
			break;
	}

}

