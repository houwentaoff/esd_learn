/*************************************************************************
	> File Name: sem.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月22日 星期三 11时05分34秒
 ************************************************************************/

#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
//临界资源
char *names[5];
int size = 0;
//定义锁，信号量
sem_t lock;

void *thread1(void * p){
	sem_wait(&lock);
	names[size] = "关兴";
	size++;
	sleep(1);
	printf("-----thread1----\n");
	sem_post(&lock);
}

void *thread2(void *p){
	sem_wait(&lock);
	names[size] = "张包";
	size++;
	sleep(1);
	printf("-----thread2------\n");
	sem_post(&lock);
}
int main(void)
{
	//初始化锁
	sem_init(&lock, 0, 1);
	pthread_t t1,t2;
	//加锁
	sem_wait(&lock);
	names[size] = "郭淮";
	size++;
	sem_post(&lock);//解锁

	pthread_create(&t1, 0, thread1, 0);
	pthread_create(&t2, 0, thread2, 0);

	pthread_join(t1, 0);
	pthread_join(t2, 0);

	//删除锁
	sem_destroy(&lock);

	int i;
	for(i=0; i<size; i++){
		printf("%s ", names[i]);
	}
	printf("\n");
}
