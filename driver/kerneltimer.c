/*************************************************************************
	> File Name: kerneltimer.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月16日 星期四 17时14分52秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

static struct timer_list bigdog_timer;
static unsigned long bark_time = 5;//time interval
static unsigned long bark_times = 50;//bark times
static int bark_count = 0;

static void bigdog_timer_handler(unsigned long data){
	if(bark_count < bark_times){
		printk("BIgdog barking times:%d\n", ++bark_count);
		//add timer again
		bigdog_timer.expires = jiffies+(bark_time*HZ);
		add_timer(&bigdog_timer);
	}
}
static int __init kerneltimer_init(void){
	init_timer(&bigdog_timer);
	bigdog_timer.expires = jiffies+(bark_time*HZ);
	bigdog_timer.function = bigdog_timer_handler;
	//add timer list to kernel
	add_timer(&bigdog_timer);

	return  0;
}


static void __exit kerneltimer_exit(void){
	del_timer(&bigdog_timer);
}
module_init(kerneltimer_init);
module_exit(kerneltimer_exit);
