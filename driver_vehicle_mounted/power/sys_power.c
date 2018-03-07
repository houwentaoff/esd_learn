/*************************************************************************
	> File Name: sys_power.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年05月19日 星期五 11时29分47秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/pinconf-sunxi.h>
#include <linux/pinctrl/consumer.h>
#include <linux/clk.h>
#include <linux/clk/sunxi.h>
#include <linux/platform_device.h>
#include <linux/power/scenelock.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>


MODULE_LICENSE("GPL");

#define DEVICE_NAME "syspower"
#define MISC_IOC_MAGIC 'O'
#define BD5V_PWREN_ON   _IO(MISC_IOC_MAGIC, 0x00)
#define BD5V_PWREN_OFF  _IO(MISC_IOC_MAGIC, 0x01)
#define BD5V_PWREN_GET  _IO(MISC_IOC_MAGIC, 0x02)
#define BD18V_PWREN_ON  _IO(MISC_IOC_MAGIC, 0x03)
#define BD18V_PWREN_OFF _IO(MISC_IOC_MAGIC, 0x04)
#define BD18V_PWREN_GET _IO(MISC_IOC_MAGIC, 0x05)
#define P5V_PWREN_ON    _IO(MISC_IOC_MAGIC, 0x06)
#define P5V_PWREN_OFF   _IO(MISC_IOC_MAGIC, 0x07)
#define P5V_PWREN_GET   _IO(MISC_IOC_MAGIC, 0x08)
#define ETH_PWREN_ON    _IO(MISC_IOC_MAGIC, 0x09)
#define ETH_PWREN_OFF   _IO(MISC_IOC_MAGIC, 0x0A)
#define ETH_PWREN_GET   _IO(MISC_IOC_MAGIC, 0x0B)
#define WIFI_BT_ON      _IO(MISC_IOC_MAGIC, 0x0C)
#define WIFI_BT_OFF     _IO(MISC_IOC_MAGIC, 0x0D)
#define WIFI_BT_GET     _IO(MISC_IOC_MAGIC, 0x0E)
#define AP_ENB2_ON      _IO(MISC_IOC_MAGIC, 0x0F)
#define AP_ENB2_OFF     _IO(MISC_IOC_MAGIC, 0x10)
#define AP_ENB2_GET     _IO(MISC_IOC_MAGIC, 0x11)

#if 0
#define GPIO0_LDO_ON  _IO(MISC_IOC_MAGIC, 0x00)
#define GPIO0_LDO_OFF _IO(MISC_IOC_MAGIC, 0x01)
#define P5V_PWREN_ON  _IO(MISC_IOC_MAGIC, 0x02)
#define P5V_PWREN_OFF _IO(MISC_IOC_MAGIC, 0x03)
#define OTG_PWREN_ON   _IO(MISC_IOC_MAGIC, 0x06)
#define OTG_PWREN_OFF  _IO(MISC_IOC_MAGIC, 0x07)
#define BD18V_PWREN_ON  _IO(MISC_IOC_MAGIC, 0x08)
#define BD18V_PWREN_OFF _IO(MISC_IOC_MAGIC, 0x09)
#endif 
#define POWER_CMD_MAXNR 0X15
#define ON  1
#define OFF 0

#define P5V_PWREN  GPIOE(8)
#define BD_PWR_EN  GPIOE(7) // 18V
#define OTG_PWREN  GPIOE(4)
#define BD5V_PWREN GPIOE(13)
#define MIPIDSI_PWREN GPIOE(18)
#define VCC_CTP_PWREN GPIOE(19)
#define ETH_PWREN     GPIOE(9)
#define AP_PWREN_B2   GPIOE(12)
#define BD_PA_IRQ      GPIOH(6) //输入引脚


static int gpio_get(unsigned gpio, const char *label){
	int level=0;
	gpio_request_one(gpio, GPIOF_IN, label);

	level = gpio_get_value(gpio);

	gpio_free(gpio);
	printk("bshui %s get level is %d\n", label, level);
	return level;
}

static int gpio_en(unsigned gpio, const char *label, int onoff){
	int ret=0;
	printk("bshui %s request onoff=%d\n", label, onoff);
	if(onoff){
		ret = gpio_request_one(gpio,GPIOF_OUT_INIT_HIGH, label);
	}else{
		ret = gpio_request_one(gpio,GPIOF_OUT_INIT_LOW, label);
	}
	
	if(ret){
		printk("bshui request gpio %s fail\n", label);
	}

	gpio_free(gpio);

	return ret;
}

static int regulator_get_ldo(char *name){
	struct regulator *ldo = NULL;
	int ret=0;
	ldo = regulator_get(NULL, name);

	ret = regulator_is_enabled(ldo);
	printk("bshui regulator_get_ldo ret=%d\n", ret);
	regulator_disable(ldo);
	regulator_put(ldo);
	if(ret>0)
	  return 1;
	else
	  return 0;
}

static int regulator_ldo(int onoff, char *name){
	struct regulator *ldo = NULL;
	ldo = regulator_get(NULL, name);
	if(onoff){
		printk("bshui regulator %s on\n", name);
		if(regulator_is_enabled(ldo)>0){
			printk("bshui regulator already enable\n");
			regulator_enable(ldo);
		}else{
			regulator_enable(ldo);
		}
	}else{
		regulator_disable(ldo);
	}

	regulator_put(ldo);

	return 0;
}

static long syspower_misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	printk("bshui syspower_misc_ioctl cmd=%d\n", cmd);

	if(_IOC_TYPE(cmd) != MISC_IOC_MAGIC)
	  return -EFAULT;
	if(_IOC_NR(cmd) > POWER_CMD_MAXNR)
	  return -EFAULT;
	switch(cmd){
		case BD5V_PWREN_ON:
			gpio_en(BD5V_PWREN, "bd5v_pwren", 1);
			break;
		case BD5V_PWREN_OFF:
			gpio_en(BD5V_PWREN, "bd5v_pwren", 0);
			break;
		case BD5V_PWREN_GET:
			return gpio_get(BD5V_PWREN, "bd5v_pwren");
		case BD18V_PWREN_ON:
			gpio_en(BD_PWR_EN, "bd18v_pwren", 1);
			break;
		case BD18V_PWREN_OFF:
			gpio_en(BD_PWR_EN, "bd18v_pwren", 0);
			break;
		case BD18V_PWREN_GET:
			return gpio_get(BD_PWR_EN, "bd18v_pwren");
		case P5V_PWREN_ON:
			gpio_en(P5V_PWREN, "p5v_pwren", 1);
			break;
		case P5V_PWREN_OFF:
			gpio_en(P5V_PWREN, "p5v_pwren", 0);
			break;
		case P5V_PWREN_GET:
			return gpio_get(P5V_PWREN, "p5v_pwren");
		case ETH_PWREN_ON:
			gpio_en(ETH_PWREN, "eth_pwren", 1);
			break;
		case ETH_PWREN_OFF:
			gpio_en(ETH_PWREN, "eth_pwren", 0);
			break;
		case ETH_PWREN_GET:
			return gpio_get(ETH_PWREN, "eth_pwren");
		case WIFI_BT_ON:
			regulator_ldo(ON,"wifi-bt");
			break;
		case WIFI_BT_OFF:
			regulator_ldo(OFF,"wifi-bt");
			break;
		case WIFI_BT_GET:
			return regulator_get_ldo("wifi-bt");
		case AP_ENB2_ON:
			gpio_en(AP_PWREN_B2, "ap_pwren_b2", 1);
			break;
		case AP_ENB2_OFF:
			gpio_en(AP_PWREN_B2, "ap_pwren_b2", 0);
			break;
		case AP_ENB2_GET:
			return gpio_get(AP_PWREN_B2,"ap_pwren_b2");

	}

	return 0;
}

static const struct file_operations syspower_misc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = syspower_misc_ioctl,
};

static struct miscdevice syspower_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sys_power",
	.fops = &syspower_misc_fops,
};

static ssize_t bd5v_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count){
	int enable;

	enable = simple_strtoul(buf, NULL,10);
	if(enable==0 || enable==1){
		gpio_en(BD5V_PWREN, "bd5v_pwren", enable);
	}


	return count;
}

static ssize_t eth_enable_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count){
	int enable;
	enable = simple_strtoul(buf, NULL,10);
	if(enable==0 || enable==1){
		gpio_en(ETH_PWREN, "eth3v3_pwren", enable);
	}


	return count;
}

static DEVICE_ATTR(enable, S_IRWXU | S_IRWXO | S_IRWXG, NULL, bd5v_enable_store);
static DEVICE_ATTR(eth_enable, S_IRWXU | S_IRWXO | S_IRWXG, NULL, eth_enable_store);

static struct attribute *sys_power_attrs[] = {
	&dev_attr_enable.attr,
	&dev_attr_eth_enable.attr,
	NULL
};

static const struct attribute_group sys_power_attr_group = {
	.attrs = sys_power_attrs,
};

static irqreturn_t bd_pa_irq_handler(int irq, void *handler){
	int level;

	level = gpio_get_value(BD_PA_IRQ);
	if(level==1){
		gpio_en(BD_PWR_EN, "bd_pwr_en", 1);
		printk("bshui bd_pa_irq_handler level=%d\n",level);
	}else if(level==0){
		printk("bshui bd_pa_irq_handler level=%d\n",level);
		gpio_en(BD_PWR_EN,"bd_pwr_en", 0);
	}


	return IRQ_HANDLED;
}

static int syspower_probe(struct platform_device *pdev){
	int err;

	regulator_ldo(ON, "vcc-gpio0");
	regulator_ldo(ON,"wifi-bt");
	gpio_en(P5V_PWREN, "p5v_power", 0);
	gpio_en(BD_PWR_EN, "bd_pwr_en", 0);
	gpio_en(OTG_PWREN, "otg_pwren", 0);
	gpio_en(BD5V_PWREN, "bd5v_pwren", 0);
	gpio_en(VCC_CTP_PWREN, "vcc_ctp_pwren", 1);
	gpio_en(ETH_PWREN, "eth_pwren", 0);
	gpio_en(AP_PWREN_B2, "ap_pwren_b2", 1);


	printk("bshui , syspower_probe\n");
	misc_register(&syspower_misc_device);

	err = sysfs_create_group(&pdev->dev.kobj,&sys_power_attr_group);
	if(err)
		printk("bshui sysfs_create_group sys_power err\n");
	//bd18v irq en 
	gpio_request_one(BD_PA_IRQ, GPIOF_IN, "gpio-pa");

	request_irq(gpio_to_irq(BD_PA_IRQ), bd_pa_irq_handler,
				IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
				"bd-pa", NULL);

	return 0;
}

static int syspower_remove(struct platform_device *pdev){
	printk("bshui syspower_remove\n");
	misc_deregister(&syspower_misc_device);
	sysfs_remove_group(&pdev->dev.kobj,&sys_power_attr_group);
	free_irq(gpio_to_irq(BD_PA_IRQ),NULL);
	gpio_free(BD_PA_IRQ);

	return 0;
}

static int syspower_suspend(struct device *dev){
	regulator_ldo(OFF, "vcc-gpio0");
	regulator_ldo(OFF,"wifi-bt");
	gpio_en(P5V_PWREN, "p5v_power", 0);
	printk("bshui syspower_suspend\n");
	return 0;
}

static int syspower_resume(struct device *dev){
	regulator_ldo(ON, "vcc-gpio0");
	regulator_ldo(ON,"wifi-bt");
	gpio_en(P5V_PWREN, "p5v_power", 1);
	printk("bshui syspower_resume\n");
	return 0;
}

static const struct dev_pm_ops syspower_ops = {
	.suspend_noirq = syspower_suspend,
	.resume_noirq  = syspower_resume,
};

static struct platform_driver syspower_driver = {
	.probe = syspower_probe,
	.remove = syspower_remove,
	.driver = {
		.name = DEVICE_NAME,
		.owner = THIS_MODULE,
		.pm=&syspower_ops,
	},
};

static struct platform_device syspower_device = {
	.name = DEVICE_NAME,
};

static int __init syspower_init(void){
	
	platform_device_register(&syspower_device);
	return platform_driver_register(&syspower_driver);
}

static void __exit syspower_exit(void){
	
	platform_driver_unregister(&syspower_driver);
	platform_device_unregister(&syspower_device);

}

module_init(syspower_init);
module_exit(syspower_exit);


