/*************************************************************************
	> File Name: thread.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月22日 星期三 10时26分58秒
 ************************************************************************/

#include<stdio.h>
#include<pthread.h>

void *create(void *arg){
	printf("new thread created\n");
}
int main(void)
{
	pthread_t tidp;
	int error;
	error = pthread_create(&tidp, NULL, create, NULL);
	if(error !=0){
		printf("pthread_create is not created...\n");
		return -1;
	}

	printf("pthread_create is created\n");
	return 0;
}
