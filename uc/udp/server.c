/*************************************************************************
	> File Name: server.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月23日 星期四 09时06分15秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main(){
	int sd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sd == -1){
		perror("socket create fail\n");
		exit(-1);
	}
	//准备地址
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = PF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.s_addr = inet_addr("10.2.11.188");
	//绑定地址
	int r = bind(sd, (const struct sockaddr*)&addr, sizeof(addr));
	if(r == -1){
		perror("bind fail\n");
		exit(-1);
	}



	//接受客户端连接
	while(1){
		struct sockaddr_in fromaddr;
		memset(&fromaddr, 0, sizeof(fromaddr));
		//接收数据
		char buf[100] = {};
		int len = sizeof(fromaddr);
		recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&fromaddr, &len);
		printf("从客户端%s收到数据:%s\n", inet_ntoa(fromaddr.sin_addr), buf);
		//发送数据
		char a[10][80];
		char c[80];
		int i;
		printf("请输入数据:\n");
		for(i=0; i<10; i++)
			gets(a[i]);
		for(i=0; i<5; i++){
			strcpy(c, a[i]);
			strcpy(a[i], a[9-i]);
			strcpy(a[9-i], c);
		}
		printf("*****************\n");
		for(i=0; i<10; i++){
			puts(a[i]);
			sendto(sd, "ok ,data is recv\n", 18, 0, (struct sockaddr*)&fromaddr, sizeof(fromaddr));
		}
		printf("\n");
		return 0;
	}
	close(sd);

}
