/*************************************************************************
	> File Name: kobject.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年06月21日 星期二 15时59分06秒
 ************************************************************************/
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>

MODULE_LICENSE("GPL")



module_init(kobj_test_init);
module_exit(kobj_test_exit);
