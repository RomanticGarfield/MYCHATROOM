#ifndef __STRUCT_H__
#define __STRUCT_H__

#define GROUPSIZE 50   // 最大群规模
#define GROUPCOUNT 20  // 最大群数量
#define BUFSIZE 1024   // 缓冲区大小
#define FRIENDSIZE 200 // 最大好友数

// 用户操作类型
#define _USERREG 1

#define _USERONL 2

#define _USEROFFL 3

#define _USERUPD 4

#define _FRIENDSHOW 5

#define _FRIENDADD 6

#define _FRIENDFIND 7

#define _CHATONE 8

#define INVALID_USERINFO 'n' // 无效信息
#define VALID_USERINFO 'y'	// 有效信息

// 好友
typedef struct{
	char name[32];
	char sex[10];
	char status;
}friend_info_t;

// 群
typedef struct{
	char groupname[32];
	char owner[32];
	int groupid;
	int size;
	friend_info_t member[GROUPSIZE];
}group_info_t;

// 日期
typedef struct{
	int year;
	int month;
	int day;
}user_date_t;

// 时间
typedef struct{
	int hour;
	int minute;
	int second;
}user_time_t;

// 一条消息
typedef struct{
	user_date_t date;
	user_time_t time;
	char send_name[32];// 收信人姓名
	char from_name[32];// 发信人姓名
	char message_buf[BUFSIZE-36];
}message_t;

// 文件
typedef struct{
	char filename[32];
	char filepath[BUFSIZE];
	int count;
	int length;
}file_info_t;

// 登录下线时间
typedef struct{
	user_date_t date;
	user_time_t time;
}user_logtime_t;


// 用户信息
typedef struct{
	int userid;          // 用户id
	char username[32];   // 用户姓名
	char userpasswd[32]; // 用户密码
	char usersex[10];    // 用户性别
	char status;         // 用户状态

	int friendlist[FRIENDSIZE]; // 用户好友列表
	int grouplist[GROUPCOUNT];  // 用户群聊列表

	int conn_fd; // 连接套接字
	user_logtime_t ontime; // 上线时间
	user_logtime_t offtime;// 下线时间
}userinfo_service_t;

// TCP数据包
typedef struct{
	userinfo_service_t userinfo;
	friend_info_t friendinfo;
	group_info_t groupinfo;
	user_date_t date;
	user_time_t time;
	message_t message;
	
	int conn_fd;
	char check_answer;
	int input_check;
	char user_status;
	int command_type;
	char buf[BUFSIZE];
}send_t;

#endif
