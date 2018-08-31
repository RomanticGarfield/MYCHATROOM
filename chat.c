// Author :         Cocoon
// Email :          Cocoon0206@163.com
// Last Modified :  2018-08-31 22:54
// Filename :		chat.c
// Description :    服务端功能函数的实现

#include "service.h"

// 用户信息写入
int userinfo_p_insert(const userinfo_service_t* data){
	
	chdir(USER_FILE);
	
	FILE *fp = open("USER", "ab");
	if(fp == NULL){
		return 0;
	}
	int ret = fwrite(data, sizeof(userinfo_service_t), 1, fp);

	fclose(fp);

	return ret;
}

// 根据用户名获取用户信息并通过data传出
int userinfo_p_selectbyname(const char* username, userinfo_service_t *data){
	chdir(USER_FILE);
	userinfo_service_t buf;
	int ok = 0;
	FILE* fp = fopen("USER","rb");
	if(fp == NULL){
		return 0;
	}
	while(!feof(fp)){
		if(fread(&buf, sizeof(userinfo_service_t), 1, fp)){
			if(strcmp(buf.username, username) == 0){
				*data = buf;
				ok = 1;
				break;
			}
		}
	}

	fclose(fp);
	return ok;
}

int serve_user_register(send_t *precv) // 注册函数
{
	// 基本信息录入
	userinfo_service_t data;
	send_t *psend;

	strcpy(data.username, precv->userinfo.username);
	strcpy(data.userpasswd, precv->userinfo.userpasswd);
	strcpy(data.usersex, precv->userinfo.usersex);
	
	// id分配
	data.userid = EntKey_Srv_CompNewKey((char*)("USER"));
	
	// 初始化
	data.status = _OFFLINE;
	for(int i = 0; i < FRIENDSIZE; i++){
		data.friendlist[i] = 0;
	}
	for(int i = 0; i < GROUPCOUNT; i++){
		data.grouplist[i] = 0;
	}

	data.ontime.date.year = 0;
	data.ontime.date.month = 0;
	data.ontime.date.day = 0;
	data.offtime.date.year = 0;
	data.offtime.date.month = 0;
	data.offtime.date.day = 0;
	data.ontime.time.hour = 0;
	data.ontime.time.minute = 0;
	data.ontime.time.second = 0;
	data.offtime.time.hour = 0;
	data.offtime.time.minute = 0;
	data.offtime.time.second = 0;
	
	// 用户信息录入
	int ret = userinfo_p_insert(&data);
	if(ret > 0){
		printf("\nNO.%d user registed, information below:\n", data.userid);
		printf("name: %s\n", data.username);
		printf("sex: %s\n", data.usersex);

		send_data(precv->conn_fd, psend);
	}

	return ret;
}



int serve_user_login(send_t *precv)  // 登录函数
{
	send_t send_buf;
	send_t *psend = &send_buf;

	userinfo_service_t info;
	userinfo_service_t *data = &info;
	
	psend->conn_fd = precv->conn_fd;

	// 用户名密码校验,通过data返回匹配用户信息
	userinfo_p_selectbyname(precv->userinfo.username, data);
	
	if(data->status == _ONLINE){//用户已经在线
		printf("<!> name [%s] sign conflict, system refused \
			sign in\n", precv->userinfo.username);
		psend->userstatus = INVALID_USERINFO;
		psend->input_check = _ONLINE;
		
		send_data(psend->conn_fd, psend);
		return 0;
	}
	//密码检验
	if(strcmp(data->userpasswd, precv->userinfo.userpasswd) == 0){
		//密码正确,允许登录
		//系统提示有新用户登录
		printf("\nNO.%d log in, the information below:\n", data->userid);
		printf("name: %s\n", data->username);
		printf("sex: %s\n", data->usersex);
		printf("last log in time:%d-%d-%d %d:%d:%d\n\n", data->ontime.date.year,\
			data->ontime.date.month, data->ontime.date.day, data->ontime.time.hour,\
			data->ontime.time.minute, data->ontime.time.second);
		
		psend->userstatus = VALID_USERINFO;
	}else{
		//密码错误
		//给客户端发送错误提示
		psend->userstatus = INVALID_USERINFO;
		psend->input_check = _INPUTPASSWD;
		
		send_data(psend->conn_fd, psend);
		return 0;
	}
	
	data->conn_fd = precv->conn_fd;
	
	//更改系统信息
	data->status = _ONLINE;
	data->ontime.date = DateNow();
	data->ontime.time = TimeNow();

	data->conn_fd = psend->conn_fd;
	userinfo_p_update(data);//保存

	send_data(psend->conn_fd, psend);
	
	return 0;
}
int serve_user_logof(send_t *precv);  // 下线函数

int serve_user_update(send_t *precv); // 修改基本信息

int serve_friend_show(send_t *precv); // 查看好友列表

int serve_friend_add(send_t *precv);  // 添加好友请求

int serve_friend_find(send_t *precv); // 查找好友

int serve_chat_one(send_t *precv);   // 聊天

