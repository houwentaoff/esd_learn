/*************************************************************************
	> File Name: mutex.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月22日 星期三 11时28分36秒
 ************************************************************************/

#include<stdio.h>
#include<pthread.h>
//临界资源
char *names[5];
int size = 0;
//定义锁，互斥量
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *thread1(void *p){
	//加锁
	pthread_mutex_lock(&lock);
	names[size] = "红太狼";
	size++;
	printf("----thread1-----\n");
	pthread_mutex_unlock(&lock);
}

void *thread2(void *p){
	pthread_mutex_lock(&lock);
	names[size] = "喜洋洋";
	size++;
	printf("-----thread2------\n");
	pthread_mutex_unlock(&lock);
}
int main(void)
{
	pthread_t t1, t2;
	//临界区
	pthread_mutex_lock(&lock);
	names[size] = "灰太狼";
	size++;
	pthread_mutex_unlock(&lock);

	pthread_create(&t1, 0, thread1, 0);
	pthread_create(&t2, 0, thread2, 0);

	pthread_join(t1, 0);
	pthread_join(t2, 0);
	//删除锁
	pthread_mutex_destroy(&lock);
	int i;
	for(i=0; i<size; i++)
		printf("%s ", names[i]);

	printf("\n");
}
