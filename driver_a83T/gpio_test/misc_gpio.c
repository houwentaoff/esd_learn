/*************************************************************************
	> File Name: misc_gpio.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年06月12日 星期日 11时09分11秒
 ************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>

#define GPIO_LED GPIO_AXP(4)
#define DEVICE_NAME "gpio_misc"

MODULE_LICENSE("GPL");

#define MISC_IOC_MAGIC 'M'
#define LED_ON   _IO(MISC_IOC_MAGIC,0x00)
#define LED_OFF  _IO(MISC_IOC_MAGIC,0x01)

#define MISC_GPIO_MAXNR 2

static long gpio_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	int ret=0;

	if(_IOC_TYPE(cmd) != MISC_IOC_MAGIC)
		  return -EFAULT;
	if(_IOC_NR(cmd) > MISC_GPIO_MAXNR)
			return -EFAULT;
	switch(cmd){
		case LED_ON:
			printk("led on\n");
			gpio_free(GPIO_LED);
			ret = gpio_request_one(GPIO_LED, GPIOF_OUT_INIT_HIGH,"ledon");
			if(ret){
			printk("request gpio fail\n");
			gpio_free(GPIO_LED);
			gpio_request_one(GPIO_LED, GPIOF_OUT_INIT_HIGH,"ledon");
			}
			gpio_free(GPIO_LED);
			break;
		case LED_OFF:
			printk("led off\n");
			ret = gpio_request_one(GPIO_LED, GPIOF_OUT_INIT_LOW,"ledoff");
			if(ret){
				
			printk("request gpio fail\n");
			gpio_free(GPIO_LED);
			gpio_request_one(GPIO_LED, GPIOF_OUT_INIT_LOW,"ledlow");
			}
			gpio_free(GPIO_LED);
			break;
	
	
	}
			

	return 0;
}

static struct file_operations gpio_misc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = gpio_misc_ioctl,
};

static struct miscdevice gpio_misc={
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &gpio_misc_fops,
};

static int __init gpio_init(void){
	printk("bshui misc gpio init\n");
	misc_register(&gpio_misc);
	return 0;
}

static void __exit gpio_exit(void){
	misc_deregister(&gpio_misc);
}
module_init(gpio_init);
module_exit(gpio_exit);
