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
	int sd = socket(PF_INET, SOCK_STREAM, 0);
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

	//监听服务
	r = listen(sd, 10);
	if( r== -1){
		perror("listen fail\n");
		exit(-1);
	}
	//接受客户端连接
	while(1){
		struct sockaddr_in fromaddr;
		memset(&fromaddr, 0, sizeof(fromaddr));
		//长度一定要指定，不能为0
		int len = sizeof(fromaddr);
		int asd = accept(sd, (struct sockaddr*)&fromaddr, &len);

		char *msg = inet_ntoa(fromaddr.sin_addr);
		printf("有客户端%s连接上来\n", msg);
		
		char buf[100] = {};
		recv(asd, buf, sizeof(buf), 0);
		printf("从客户端%s接收到的数据:%s\n", inet_ntoa(fromaddr.sin_addr),
				buf);
		send(asd, "很好,OK\n", strlen("很好,OK\n"), 0);
	}
	close(sd);

}
