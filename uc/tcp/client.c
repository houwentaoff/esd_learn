/*************************************************************************
	> File Name: client.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月23日 星期四 09时28分52秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main(void)
{
	int sd = socket(PF_INET, SOCK_STREAM, 0);
	if(sd == -1){
		perror("socket fail\n");
		exit(-1);
	}

	//准备地址
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = PF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.s_addr = inet_addr("10.2.11.188");
	//连接地址
	int r = connect(sd, (const struct sockaddr*)&addr, sizeof(addr));
	if(r == -1){
		perror("connect fail");
		exit(-1);
	}
	//和服务端进行通信
	char buf[100] = "hello server";
	send(sd, buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	recv(sd, buf, sizeof(buf), 0);
	printf("server said:%s\n", buf);

	close(sd);
}
