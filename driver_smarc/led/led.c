/*************************************************************************
	> File Name: misc_sketech.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年11月03日 星期五 10时37分08秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>

#define DRIVER_NAME "flash_led"
#define DEVICE_NAME "flash_led"
#define MISC_NAME   "led"

#define MISC_IOC_MAGIC 'M'
#define LED_ON _IO(MISC_IOC_MAGIC, 0x01)
#define LED_OFF _IO(MISC_IOC_MAGIC, 0x02)

struct flash_led_data {
	unsigned int gpio;
};
static int gpio = 0;

static long flash_led_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	printk("flash_led misc ioctl cmd=%d\n", cmd);
	if(_IOC_TYPE(cmd)!= MISC_IOC_MAGIC)
	  return -EFAULT;

	switch(cmd){
		case LED_ON:
			gpio_set_value(gpio, 1);
			break;
		case LED_OFF:
			gpio_set_value(gpio, 0);
			break;
		default:
			break;
	}

	return 0;
}

static const struct file_operations flash_led_misc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = flash_led_misc_ioctl,
};

static struct miscdevice flash_led_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = MISC_NAME,
	.fops  = &flash_led_misc_fops,
};


static int flash_led_probe(struct platform_device *pdev){
	struct device_node *np = pdev->dev.of_node;
	struct flash_led_data *data;
	int ret = 0;

	if(np == NULL){
		return 0;//void the probe exec twice
	}
	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if(data == NULL)
		return -ENOMEM;

	data->gpio = of_get_named_gpio(np,"gpios", 0);
	gpio = data->gpio;

	ret = devm_gpio_request_one(&pdev->dev, data->gpio,GPIOF_OUT_INIT_LOW,NULL);
	if(ret < 0)
	  printk("bshui failed to request flash led gpio\n");
	misc_register(&flash_led_misc_device);

	printk("bshui flash_led_probe-------\n");
	return 0;
}

static int flash_led_remove(struct platform_device *pdev){
	struct device_node *np = pdev->dev.of_node;
	if(np == NULL)
	  return 0;

	misc_deregister(&flash_led_misc_device);
	return 0;
}

static int flash_led_suspend(struct platform_device *pdev, pm_message_t state){

	return 0;
}

static int flash_led_resume(struct platform_device *pdev)
{
	return 0;
}

static void flash_led_device_release(struct device *pdev){

	return ;
}

static struct of_device_id flash_dt_ids[] = {
	{ .compatible = "flash-leds"},
	{}
};

MODULE_DEVICE_TABLE(of, flash_dt_ids);

static struct platform_driver flash_led_driver = {
	.probe  = flash_led_probe,
	.remove = flash_led_remove,
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(flash_dt_ids),
	},
	.suspend = flash_led_suspend,
	.resume  = flash_led_resume,
};


static struct platform_device flash_led_device = {
	.name = DEVICE_NAME,
	.dev = {
		.release = flash_led_device_release,
	}
};

static int __init flash_led_init(void){
	platform_device_register(&flash_led_device);
	platform_driver_register(&flash_led_driver);
	return 0;
}

static void __exit flash_led_exit(void){
	platform_driver_unregister(&flash_led_driver);
	platform_device_unregister(&flash_led_device);
}

module_init(flash_led_init);
module_exit(flash_led_exit);

MODULE_LICENSE("GPL");
