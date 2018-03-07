/*************************************************************************
	> File Name: call_usermodehelper_test.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年11月19日 星期三 15时18分06秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/sched.h>
MODULE_LICENSE("GPL");

static __init int call_usermode_init(void){
	int result = 0;
	char cmdPath[]="/sbin/ifconfig";
	char *cmdArgv[]={cmdPath, "eth0 10.2.11.115", NULL};
	char *cmdEnvp[]={"HOME=/", "PATH=/sbin=:/bin:/usr/bin", NULL};

	result = call_usermodehelper(cmdPath, cmdArgv, cmdEnvp, UMH_WAIT_PROC);
	printk("result=%d\n", result);

	return result;

}
static __exit void call_usermode_exit(void){

}
module_init(call_usermode_init);
module_exit(call_usermode_exit);
