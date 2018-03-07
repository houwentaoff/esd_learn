/*************************************************************************
	> File Name: thread_creat.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年09月04日 星期一 09时21分27秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct member {
	int a;
	char *s;
};

void *create(void *arg){
	struct member *temp;
	temp = (struct member*)arg;
	printf("member->a=%d\n", temp->a);
	printf("member->s=%s\n", temp->s);
	

	return (void *)0;
}

int main(void)
{
	pthread_t tidp;
	int error;
	struct member *b;

	b= (struct member*)malloc(sizeof(struct member));
	b->a = 4;
	b->s = "bshui";

	error = pthread_create(&tidp, NULL, create, (void*)b);
	if(error){
		printf("pthrea is not created\n");
		return -1;
	}

	sleep(1);
	printf("pthread is created\n");

	return 0;
}
