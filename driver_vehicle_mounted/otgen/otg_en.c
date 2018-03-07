/*************************************************************************
	> File Name: otg_en.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年06月09日 星期五 14时14分34秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <mach/sys_config.h>
#include <linux/platform_device.h>

#define OTG_PWREN GPIOE(4)

static usb_id_gpio = 0;


static irqreturn_t otgen_irq_handler(int irq, void *handler){

	int level;
	level = gpio_get_value(usb_id_gpio);
	//printk("bshui %s level=%d \n", __func__, level);
	if(level == 0){
		//set the otg_en low
		gpio_request_one(OTG_PWREN, GPIOF_OUT_INIT_LOW, "otg_pwren");
	}else if(level == 1){
	
		gpio_request_one(OTG_PWREN, GPIOF_OUT_INIT_HIGH, "otg_pwren");
	}

	gpio_free(OTG_PWREN);
	return IRQ_HANDLED;
}

static int otgen_probe(struct platform_device *pdev){
	script_item_value_type_e type = 0;
	script_item_u item_temp;
	int ret = 0;

	type = script_get_item("usbc0","usb_id_gpio",&item_temp);
	if(SCIRPT_ITEM_VALUE_TYPE_PIO != type){
		return -1;
	}else{
		usb_id_gpio = item_temp.gpio.gpio;
	}

	gpio_request_one(usb_id_gpio, GPIOF_IN, "usb-id");

	ret = request_irq(gpio_to_irq(usb_id_gpio), otgen_irq_handler,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
				"otgen", NULL);
	if(ret < 0){
		printk("bshui failed to register irq otg_en\n");
		return -1;
	}


}

static int otgen_remove(struct platform_device *pdev){
	free_irq(gpio_to_irq(usb_id_gpio), NULL);
	gpio_free(usb_id_gpio);

	return 0;
}

static struct platform_driver otgen_driver = {
	.probe = otgen_probe,
	.remove = otgen_remove,
	.driver = {
		.name = "otgen",
		.owner = THIS_MODULE,
	},
};

static struct platform_device otgen_dev = {
	.name = "otgen",
};

static int __init otgen_init(void){
	platform_device_register(&otgen_dev);
	platform_driver_register(&otgen_driver);
	return 0;
}

static void __exit otgen_exit(void){
	platform_driver_unregister(&otgen_driver);
	platform_device_unregister(&otgen_dev);
}

module_init(otgen_init);
module_exit(otgen_exit);
MODULE_LICENSE("GPL");
