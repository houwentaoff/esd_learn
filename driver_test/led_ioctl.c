/*************************************************************************
	> File Name: led_ioctl.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2015年06月09日 星期二 13时21分09秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include "led.h"
MODULE_LICENSE("GPL");
#define DEVICE_NAME "led"

static void led_tasklet_func(unsigned long data){
	printk("entry led tasklet func\n");
}

DECLARE_TASKLET(led_tasklet, led_tasklet_func, 0);
int led_irq = 11;
static long led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	return 0;
}

static struct file_operations led_fops={
	.owner = THIS_MODULE,
	.unlocked_ioctl = led_ioctl,
};

static struct miscdevice led_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &led_fops,
};

static irqreturn_t led_isr(int irq, void *dev_id){
	printk("entry led_isr\n");

	tasklet_schedule(&led_tasklet);
	return IRQ_HANDLED;
}

static int __init led_init(void){
	int ret = 0;
	misc_register(&led_misc);
	ret = request_irq(led_irq, led_isr, IRQF_DISABLED, "ledinterrupt", NULL );
	if(ret <0)
		printk("request irq failure\n");
	return 0;
}
static void __exit led_exit(void){
	free_irq(led_irq, NULL);
	misc_deregister(&led_misc);
}

module_init(led_init);
module_exit(led_exit);
