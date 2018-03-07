/*************************************************************************
	> File Name: powersave.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年10月14日 星期五 11时23分22秒
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
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/clk/sunxi.h>
#include <linux/platform_device.h>
#include <linux/power/scenelock.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "powersave"

#define GPIO_BACKLIGHT_PWM GPIOD(28)
#define GPIO_LCD_ENP GPIOD(27)
#define GPIO_LCD_ENN GPIOD(29)
#define GPIO_CAM_PWDN  GPIOE(17)
#define GPIO_CAM_RST   GPIOE(16)
#define GPIO_JD GPIOH(10)

static int gpio0_power(int onoff){
	struct regulator *ldo = NULL;
	int ret = 0;
	ldo = regulator_get(NULL, "vcc-ctp");

	if(onoff){
		ret = regulator_enable(ldo);
		printk("bshui enable ret=%d \n", ret);
	}else{
		ret = regulator_disable(ldo);
		printk("bshui disable ret=%d \n", ret);
	}
}

static int module_power(int onoff){
	struct regulator *ldo = NULL;
	int ret = 0;
	ldo = regulator_get(NULL, "iovdd-csi");

	if(IS_ERR(ldo)){
		printk("bshui some error happen, failed to get regulator\n");
		return -1;
	}
	if(onoff){
			printk("bshui regulator is enable\n");
		ret = regulator_enable(ldo);
	}else{
		ret = regulator_disable(ldo);
		ret = regulator_disable(ldo);
	}
	printk("ret=%d------------\n", ret);

	return 0;
}
static int camera_power(int onoff){
	if(onoff == 1){
		
		gpio_request_one(GPIO_CAM_RST, GPIOF_OUT_INIT_HIGH, "cam_rst");
		gpio_request_one(GPIO_CAM_PWDN, GPIOF_OUT_INIT_HIGH, "cam_pwdn");
	}else if(onoff == 0){
	
		gpio_request_one(GPIO_CAM_RST, GPIOF_OUT_INIT_LOW, "cam_rst");
		gpio_request_one(GPIO_CAM_PWDN, GPIOF_OUT_INIT_LOW, "cam_pwdn");
	}
	gpio_free(GPIO_CAM_RST);
	gpio_free(GPIO_CAM_PWDN);
	return 0;
}

static int lcd_power(int onoff){
	if(onoff == 1){
		
		gpio_request_one(GPIO_LCD_ENP, GPIOF_OUT_INIT_HIGH, "lcd_enp");
		gpio_request_one(GPIO_LCD_ENN, GPIOF_OUT_INIT_HIGH, "lcd_enn");
	}else if(onoff == 0){
	
		gpio_request_one(GPIO_LCD_ENP, GPIOF_OUT_INIT_LOW, "lcd_enp");
		gpio_request_one(GPIO_LCD_ENN, GPIOF_OUT_INIT_LOW, "lcd_enn");
	}
	gpio_free(GPIO_LCD_ENP);
	gpio_free(GPIO_LCD_ENN);
	return 0;
}

static int sys_power(int onoff){
	int ret;
	if(onoff== 1){
		
		ret = gpio_request_one(GPIO_JD, GPIOF_OUT_INIT_HIGH, "JD");
		printk("bshui suspend ret=%d\n", ret);
		if(ret){
			gpio_free(GPIO_JD);
		ret = gpio_request_one(GPIO_JD, GPIOF_OUT_INIT_HIGH, "JD");
		printk("bshui 2 suspend ret=%d\n", ret);

		}
	}else if(onoff == 0){
	
		gpio_request_one(GPIO_JD, GPIOF_OUT_INIT_LOW, "JD");
	}

		gpio_free(GPIO_JD);
	return 0;
}

static int backlight_power(int onoff){
	if(onoff== 1){
		
		gpio_request_one(GPIO_BACKLIGHT_PWM, GPIOF_OUT_INIT_HIGH, "pwm_backlight");
	}else if(onoff == 0){
	
		gpio_request_one(GPIO_BACKLIGHT_PWM, GPIOF_OUT_INIT_LOW, "pwm_backlight");
	}

		gpio_free(GPIO_BACKLIGHT_PWM);
	return 0;
}
static int powersave_probe(struct platform_device *pdev){
	printk("--bshui-----%s-----1--\n", __func__);
	//module_power(0);
//	lcd_power(1);
//	backlight_power(1);
	gpio0_power(1);

	return 0;
}

static int powersave_remove(struct platform_device *pdev){
	return 0;
}

static int powersave_suspend(struct device *dev){
//	camera_power(0);
	//backlight_power(0);
//	lcd_power(0);
	//module_power(1);
	//msleep(1000);
	printk("bshui sys_power suspend\n");
	gpio0_power(0);
	//sys_power(0);
	return 0;
}

static int powersave_resume(struct device *dev){
//	lcd_power(1);
//	camera_power(1);
	//backlight_power(1);
	printk("bshui sys_power resume\n");
//	sys_power(1);
	gpio0_power(1);
	return 0;
}
static const struct dev_pm_ops powersave_ops = {
	.suspend_noirq = powersave_suspend,
	.resume_noirq  = powersave_resume,
};
static struct platform_driver powersave_driver = {
	.probe = powersave_probe,
	.remove = powersave_remove,
	.driver = {
		.name = DEVICE_NAME,
		.owner = THIS_MODULE,
		.pm = &powersave_ops,
	},
};

static struct platform_device powersave_dev = {
	.name = DEVICE_NAME,
};

static int __init powersave_init(void){
	platform_device_register(&powersave_dev);
	return platform_driver_register(&powersave_driver);
}

static void __exit powersave_exit(void){
	platform_driver_unregister(&powersave_driver);
	platform_device_unregister(&powersave_dev);
}

module_init(powersave_init);
module_exit(powersave_exit);
