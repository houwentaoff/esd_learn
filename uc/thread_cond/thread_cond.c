/*************************************************************************
	> File Name: thread_cond.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年05月10日 星期二 15时15分32秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *thread1(void *);
void *thread2(void *);
int i=1;
int main(void)
{
	pthread_t t_a;
	pthread_t t_b;
	
	pthread_create(&t_a, 0, &thread1, 0);
	pthread_create(&t_b, 0, &thread2, 0);

	//等待线程结束 
	pthread_join(t_a, 0);
	pthread_join(t_b, 0);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	printf("-----------\n");
	exit(0);

}

void *thread1(void *junk){
	for(i=1; i<=6; i++){
		pthread_mutex_lock(&mutex);//锁住互斥量
		printf("thread1:lock %d\n", __LINE__);
		if(i%3 == 0){
			printf("thread1:signal 1 i=%d\n", i);
			pthread_cond_signal(&cond);//条件改变,发送信号,通知t_b进程 
			printf("thread1:signal 2 i=%d\n", i);
			sleep(1);
		}
		pthread_mutex_unlock(&mutex);
		printf("thread1:unlock %d\n", __LINE__);
		sleep(1);
	}
}

void *thread2(void *junk){
	while(i<=6){
		pthread_mutex_lock(&mutex);
		printf("thread2:lock %d\n", __LINE__);
		if(i%3 !=0){
			printf("thread2: wait 1 i=%d\n", i);
			pthread_cond_wait(&cond, &mutex);//等待cond改变
			printf("thread2: wait 2 i=%d\n", i);
		}
		pthread_mutex_unlock(&mutex);
		printf("thread2: unlock %d\n", __LINE__);
		sleep(1);
	}
}


