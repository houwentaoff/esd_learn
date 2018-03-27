/*************************************************************************
	> File Name: ad_hx710.c
	> Author: bshui
	> Mail: shui6666@126.com 
	> Created Time: 2017年11月03日 星期五 10时37分08秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>

#define DRIVER_NAME "ad_hx710"
#define MISC_NAME   "adhx710_misc"

#define MISC_IOC_MAGIC 'M'
#define CMD_GET_AD _IO(MISC_IOC_MAGIC, 0x01)
#define XXX_2 _IO(MISC_IOC_MAGIC, 0x02)

#define AD_BIT_NUM 24
#define AD_VALID_BIT_NUM 19

static int ad_scl=0;
static int ad_sda=0;
struct device *g_dev;

static int hx710_get_ad(void);

static long ad_hx710_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	int value=0;
	printk("ad_hx710 misc ioctl cmd=%d\n", cmd);
	if(_IOC_TYPE(cmd)!= MISC_IOC_MAGIC)
	  return -EFAULT;

	switch(cmd){
		case CMD_GET_AD:
		value = hx710_get_ad();
		printk("bshui : value:%d\n", value);
		break;
		default:
			break;
	}

	return 0;
}

static int ad_hx710_misc_read(struct file *filp,char *buf, size_t count,
			loff_t *f_ops){
	int val=0,i=0;
	val = hx710_get_ad();
	printk("bshui read val:%d\n", val);
	if(val < 0)
	  return -1;
	for(i=0; i<4; i++){
		buf[i] = val & 0xff;
		val >>=8;
	}
	//返回８应用做判断
	return 8;
}

static const struct file_operations ad_hx710_misc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = ad_hx710_misc_ioctl,
	.read = ad_hx710_misc_read,
};

static struct miscdevice ad_hx710_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = MISC_NAME,
	.fops  = &ad_hx710_misc_fops,
};
static int hx710_init(struct device *dev){
	//gpio init
	devm_gpio_request_one(dev,ad_sda, GPIOF_OUT_INIT_HIGH,"sda");
	devm_gpio_request_one(dev,ad_scl, GPIOF_OUT_INIT_LOW,"scl");
	udelay(5);
	
	return 0;
}

static int hx710_get_ad(void){
	int level=0,cnt=0,i;
	int ad_val = 0;
	gpio_set_value(ad_sda, 1);//每次获取ad之前设置sda:1,scl:0
	gpio_set_value(ad_scl, 0);
	udelay(5);
	gpio_direction_input(ad_sda);
	level = gpio_get_value(ad_sda);
	//高电平表示ad没有准备好数据
	while(level){
		mdelay(10);
		if(++cnt >10)
		  return -1;
	}
	//读出ad的24位数据
	for(i=0;i<24;i++){
		gpio_set_value(ad_scl,1);
		udelay(1);
		ad_val <<= 1;
		gpio_set_value(ad_scl,0);
		udelay(1);
		if(gpio_get_value(ad_sda))
		  ad_val++;
	}
	gpio_set_value(ad_scl, 1);
	ad_val ^= 0x800000;
	gpio_set_value(ad_scl, 0);
	//转换为内码值
	ad_val = ((ad_val >> (AD_BIT_NUM - AD_VALID_BIT_NUM))&((1<<AD_VALID_BIT_NUM)-1));
	return ad_val;
}

static int ad_hx710_probe(struct platform_device *pdev){
	struct device_node *dev_node = NULL;
	int ret=0;

	dev_node = of_find_compatible_node(NULL,NULL,"ad_hx710");
	if(!of_device_is_compatible(dev_node, "ad_hx710")){
		
		return -1;
	}
	
	ad_scl = of_get_named_gpio(dev_node, "ad1_scl", 0);
	ad_sda = of_get_named_gpio(dev_node, "ad1_sda", 0);

	hx710_init(&pdev->dev);


	printk("bshui ad_hx710_probe-------\n");


	misc_register(&ad_hx710_misc_device);

	return 0;
}

static int ad_hx710_remove(struct platform_device *pdev){

	misc_deregister(&ad_hx710_misc_device);
	printk("bshui ad_hx710_remove\n");
	return 0;
}

static int ad_hx710_suspend(struct platform_device *pdev, pm_message_t state){

	return 0;
}

static int ad_hx710_resume(struct platform_device *pdev)
{
	return 0;
}


static struct of_device_id ad_dt_ids[] = {
	{ .compatible = "ad_hx710"},
	{}
};

MODULE_DEVICE_TABLE(of, ad_dt_ids);

static struct platform_driver ad_hx710_driver = {
	.probe  = ad_hx710_probe,
	.remove = ad_hx710_remove,
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ad_dt_ids),
	},
	.suspend = ad_hx710_suspend,
	.resume  = ad_hx710_resume,
};



static int __init ad_hx710_init(void){
	platform_driver_register(&ad_hx710_driver);
	return 0;
}

static void __exit ad_hx710_exit(void){
	platform_driver_unregister(&ad_hx710_driver);
}

module_init(ad_hx710_init);
module_exit(ad_hx710_exit);

MODULE_LICENSE("GPL");
