/*************************************************************************
	> File Name: server.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年03月25日 星期五 15时17分04秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int server_sockfd, client_sockfd;
	int server_len, client_len;

	struct sockaddr_un server_address;//UNIX域套接字结构
	struct sockaddr_un client_address;

	int i, bytes;
	char ch_send, ch_recv;

	unlink("server_socket");//删除原有server_socket对象
	//创建socket,通信协议为AF_UNIX
	server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(server_sockfd == -1)
		printf("create server sockfd fail\n");

	//配置服务器信息(通信协议)
	server_address.sun_family = AF_UNIX;
	//配置服务器信息socket对象
	strcpy(server_address.sun_path, "server_socket");
	//配置服务器信息服务器地址长度
	server_len = sizeof(server_address);

	//绑定socket对象
	bind(server_sockfd, (struct sockaddr*)&server_address, server_len);
	//监听网络,队列数为5
	listen(server_sockfd, 5);
	printf("Server is waiting for client connect...\n");
	client_len = sizeof(client_address);
	//接受客户端请求
	client_sockfd = accept(server_sockfd, (struct sockaddr*)&server_address,
			(socklen_t *)&client_len);

	if(client_sockfd == -1){
		perror("accept");
		exit(EXIT_FAILURE);
	}
	printf("The server is waiting for client data...\n");

	for(i=0,ch_send='1'; i<5; i++,ch_send++){
		if((bytes=read(client_sockfd, &ch_recv, 1)) == -1){
			perror("read");
			exit(EXIT_FAILURE);
		}
		printf("The character receiver from client is %c\n", ch_recv);
		sleep(1);

		if((bytes=write(client_sockfd, &ch_send, 1))==-1){
			perror("read");
			exit(EXIT_FAILURE);
		}
	}
	close(client_sockfd);
	unlink("server_socket");
	return 0;
}
