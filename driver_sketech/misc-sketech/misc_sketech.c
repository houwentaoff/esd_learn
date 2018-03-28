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
#define MISC_NAME   "AAA_BBB"

#define MISC_IOC_MAGIC 'M'
#define XXX_1 _IO(MISC_IOC_MAGIC, 0x01)
#define XXX_2 _IO(MISC_IOC_MAGIC, 0x02)

struct AAA_BBB_data {
	unsigned int gpio;
};

static long AAA_BBB_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	printk("AAA_BBB misc ioctl cmd=%d\n", cmd);
	if(_IOC_TYPE(cmd)!= MISC_IOC_MAGIC)
	  return -EFAULT;

	switch(cmd){
	
		default:
			break;
	}

	return 0;
}

static const struct file_operations AAA_BBB_misc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = AAA_BBB_misc_ioctl,
};

static struct miscdevice AAA_BBB_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = MISC_NAME,
	.fops  = &AAA_BBB_misc_fops,
};


static int AAA_BBB_probe(struct platform_device *pdev){
	struct AAA_BBB_data *data;
	
	printk("bshui AAA_BBB_probe-------\n");
	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if(data == NULL)
		return -ENOMEM;

	misc_register(&AAA_BBB_misc_device);

	return 0;
}

static int AAA_BBB_remove(struct platform_device *pdev){

	misc_deregister(&AAA_BBB_misc_device);
	printk("bshui AAA_BBB_remove\n");
	return 0;
}

static int AAA_BBB_suspend(struct platform_device *pdev, pm_message_t state){

	return 0;
}

static int AAA_BBB_resume(struct platform_device *pdev)
{
	return 0;
}


static struct of_device_id AAA_dt_ids[] = {
	{ .compatible = "ad_hx710"},
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



static int __init AAA_BBB_init(void){
	platform_driver_register(&AAA_BBB_driver);
	return 0;
}

static void __exit AAA_BBB_exit(void){
	platform_driver_unregister(&AAA_BBB_driver);
}

module_init(AAA_BBB_init);
module_exit(AAA_BBB_exit);

MODULE_LICENSE("GPL");
