/*************************************************************************
	> File Name: hello.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年06月09日 星期二 09时31分35秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/timer.h>
MODULE_LICENSE("GPL");
//原子变量
atomic_t av;

static void __exit hello_exit(void){
	printk("hello_exit");
}

static int __init hello_init(void){
	
	return 0;
}
module_init(hello_init);
module_exit(hello_exit);
