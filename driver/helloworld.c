/*************************************************************************
	> File Name: helloworld.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月16日 星期四 15时23分12秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
	printk("hello init\n");
	return 0;
}
static void __exit hello_exit(void)
{
	printk("hello exit\n");
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
