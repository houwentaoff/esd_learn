/*************************************************************************
	> File Name: gpio_uhf.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年11月03日 星期四 16时43分30秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define RLM600_NAME "rlm600"
#define RLM600_MISC_NAME "gpio_uhf"
#define MISC_IOC_MAGIC 'N'
#define RLM600_ON  _IO(MISC_IOC_MAGIC, 0x01)
#define RLM600_OFF _IO(MISC_IOC_MAGIC, 0x00)
#define RLM600_CMD_MAXNR 2
#define RLM600_GPIO GPIOE(18)

static int rlm600_power(int onoff){
	if(onoff == 1){
		gpio_request_one(RLM600_GPIO, GPIOF_OUT_INIT_HIGH, "rlm600_en");
	}else if(onoff == 0){
		gpio_request_one(RLM600_GPIO, GPIOF_OUT_INIT_LOW, "rlm600_en");
	}
	gpio_free(RLM600_GPIO);

	return 0;
}
static long rlm600_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	printk("rlm600 misc ioctl cmd = %d\n", cmd);
	if(_IOC_TYPE(cmd) != MISC_IOC_MAGIC)
	  return -EFAULT;
	if(_IOC_NR(cmd) > RLM600_CMD_MAXNR)
	  return -EFAULT;
	switch(cmd){
		case RLM600_ON:
			rlm600_power(1);
			break;
		case RLM600_OFF:
			rlm600_power(0);

			break;
	}

	return 0;
}

static const struct file_operations rlm600_misc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = rlm600_misc_ioctl,
};

static struct miscdevice rlm600_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = RLM600_MISC_NAME,
	.fops = &rlm600_misc_fops,
};

static int rlm600_probe(struct platform_device *pdev){

	misc_register(&rlm600_misc_device);
	rlm600_power(1);
	return 0;
}

static int rlm600_remove(struct platform_device *pdev){
	misc_deregister(&rlm600_misc_device);
	return 0;
}

static int rlm600_suspend(struct platform_device *dev, pm_message_t state){
	rlm600_power(0);
	return 0;
}

static int rlm600_resume(struct platform_device *dev){
	rlm600_power(1);
	return 0;
}
static struct platform_driver rlm600_driver = {
	.probe = rlm600_probe,
	.remove = rlm600_remove,
	.driver = {
		.name = RLM600_NAME,
		.owner = THIS_MODULE,
	},
	.suspend = rlm600_suspend,
	.resume = rlm600_resume,
};

static struct platform_device rlm600_dev = {
	.name = RLM600_NAME,
};

static int __init rlm600_init(void){
	platform_device_register(&rlm600_dev);
	platform_driver_register(&rlm600_driver);

	return 0;
}

static void __exit rlm600_exit(void){
	platform_driver_unregister(&rlm600_driver);
	platform_device_unregister(&rlm600_dev);
}

module_init(rlm600_init);
module_exit(rlm600_exit);
MODULE_LICENSE("GPL");
