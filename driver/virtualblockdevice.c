/*************************************************************************
	> File Name: virtualblockdevice.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月19日 星期日 16时10分37秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");

#define VIRTUALBLOCKDEVICE_MAJOR 0
#define VIRTUALBLOCKDEVICE_MINOR 0
#define VIRTUALBLOCKDEVICE_NAME "virblkdev0"

static u32 virtualblockdevice_major = VIRTUALBLOCKDEVICE_MAJOR;
static u32 virtualblockdevice_minor = VIRTUALBLOCKDEVICE_MINOR;

static int __init virtualblockdevice_init(void){
	int ret = 0;
	printk("entry virtualblockdev init\n");
	//register block device
	ret = register_blkdev(virtualblockdevice_major, VIRTUALBLOCKDEVICE_NAME);
	if(ret < 0){
		printk("failure to register block device\n");
		goto failure_register_blkdev;
	}

	virtualblockdevice_major = ret;
	ret = 0;
	printk("success to register block device\n");

	return ret;

failure_register_blkdev:
	return ret;
}

static void __exit virtualblockdevice_exit(void){
	unregister_blkdev(virtualblockdevice_major, VIRTUALBLOCKDEVICE_NAME);
}
module_init(virtualblockdevice_init);
module_exit(virtualblockdevice_exit);
