/*************************************************************************
	> File Name: batterychange.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年11月30日 星期三 11时09分27秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <mach/sys_config.h>
#include <asm/uaccess.h>

#define BCHANGE_NAME "batterychange"
#define WORK_DELAY_TIME 100
static int tp_int_irq ;
static int bchange_gpio;

static void bchange_update(struct work_struct *work);
static DECLARE_DELAYED_WORK(bchange_work, bchange_update);
struct device *dev;
static int b_state = 2;

static void tp_enable(int onoff){
	if(onoff == 1){
		enable_irq(tp_int_irq);
	}else if(onoff == 0){
		disable_irq(tp_int_irq);
	}
}

static void bchange_update(struct work_struct *work){
	int state;
	char *envp[2];
	state = gpio_get_value(bchange_gpio);
	//printk("bchange work state=%d\n", state);
	if(b_state != state){
	  b_state = state;

		if(state == 0){
			envp[0] = "SWITCH_STATE=OPEN";
			tp_enable(0);
		}else if(state == 1){
			tp_enable(1);
			envp[0] = "SWITCH_STATE=CLOSE";
		}

		envp[1] = NULL;
		kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp);
	}

	schedule_delayed_work(&bchange_work, WORK_DELAY_TIME);
	
}
static int bchange_probe(struct platform_device *pdev){

	dev = &pdev->dev;
	gpio_request_one(bchange_gpio, GPIOF_IN, BCHANGE_NAME);
	
	schedule_delayed_work(&bchange_work, WORK_DELAY_TIME);

	return 0;
}

static int __devexit bchange_remove(struct platform_device *pdev){

	gpio_free(bchange_gpio);	
	cancel_delayed_work_sync(&bchange_work);
	return 0;
}

static void bchange_dev_release(struct device *dev){

}
static struct platform_driver bchange_driver = {
	.driver = {
		.name = BCHANGE_NAME,
		.owner = THIS_MODULE,
	},
	.probe = bchange_probe,
	.remove = __devexit_p(bchange_remove),

};

static int get_systempara(void){
	script_item_u val;
	script_item_value_type_e type;

	type = script_get_item("backbattery_para","backbattery_used", &val);
	if(SCIRPT_ITEM_VALUE_TYPE_INT != type){
		return -1;
	}
	if(val.val !=1)
	  return -1;
	type = script_get_item("backbattery_para", "back_gpio", &val);
	if(SCIRPT_ITEM_VALUE_TYPE_PIO != type){
		return -1;
	}

	bchange_gpio = val.val;

	type = script_get_item("ctp_para", "ctp_int_port", &val);
	if(SCIRPT_ITEM_VALUE_TYPE_PIO != type){
		return -1;
	}

	tp_int_irq = gpio_to_irq(val.val);

	return 0;
}

static struct platform_device bchange_dev = {
	.name = BCHANGE_NAME,
	.dev = {
		.release = bchange_dev_release,
	}
};

static int __init bchange_init(void){
	int ret;
	ret = get_systempara();
	if(!ret){
		platform_device_register(&bchange_dev);
		platform_driver_register(&bchange_driver);
	}
	return 0;
}

static void __exit bchange_exit(void){
	platform_driver_unregister(&bchange_driver);
	platform_device_unregister(&bchange_dev);
}
module_init(bchange_init);
module_exit(bchange_exit);
MODULE_LICENSE("GPL");

