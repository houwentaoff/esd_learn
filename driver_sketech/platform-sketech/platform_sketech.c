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
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>

#define DRIVER_NAME "AAA_BBB"
#define DEVICE_NAME "AAA_BBB"

struct AAA_BBB_data {
	unsigned int gpio;
};

static int AAA_BBB_probe(struct platform_device *pdev){
	struct device_node *np = pdev->dev.of_node;
	struct AAA_BBB_data *data;
	
	if(np == NULL){
		return 0;//void the probe exec twice
	}
	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if(data == NULL)
		return -ENOMEM;

	printk("bshui AAA_BBB_probe-------\n");
	return 0;
}

static int AAA_BBB_remove(struct platform_device *pdev){
	
	return 0;
}

static int AAA_BBB_suspend(struct platform_device *pdev, pm_message_t state){

	return 0;
}

static int AAA_BBB_resume(struct platform_device *pdev)
{
	return 0;
}

static void AAA_BBB_device_release(struct device *pdev){

	return ;
}

static struct of_device_id AAA_dt_ids[] = {
	{ .compatible = "flash-leds"},
	{}
};

MODULE_DEVICE_TABLE(of, AAA_dt_ids);

static struct platform_driver AAA_BBB_driver = {
	.probe  = AAA_BBB_probe,
	.remove = AAA_BBB_remove,
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(AAA_dt_ids),
	},
	.suspend = AAA_BBB_suspend,
	.resume  = AAA_BBB_resume,
};


static struct platform_device AAA_BBB_device = {
	.name = DEVICE_NAME,
	.dev = {
		.release = AAA_BBB_device_release,
	}
};

static int __init AAA_BBB_init(void){
	platform_device_register(&AAA_BBB_device);
	platform_driver_register(&AAA_BBB_driver);
	return 0;
}

static void __exit AAA_BBB_exit(void){
	platform_driver_unregister(&AAA_BBB_driver);
	platform_device_unregister(&AAA_BBB_device);
}

module_init(AAA_BBB_init);
module_exit(AAA_BBB_exit);

MODULE_LICENSE("GPL");
