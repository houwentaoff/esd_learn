/*************************************************************************
	> File Name: rtc-sd2068.c
	> Author: bshui
	> Mail: shui6666@126.com 
	> Created Time: 2018年3月27日 星期一 10时31分57秒
 ************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/rtc.h>
#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#define DRIVER_NAME "rtc_sd2068"
#define SD2068_REG_SECONDS 0x00
#define SD2068_REG_MINUTES 0x01
#define SD2068_REG_HOURS   0x02
#define SD2068_REG_WEEK    0x03
#define SD2068_REG_DAY     0x04
#define SD2068_REG_MONTH   0x05
#define SD2068_REG_YEAR    0x06


static struct i2c_client *g_client_rtc;
struct rtc_device *rtc;


static int sd2068_rtc_read_time(struct device *dev, struct rtc_time *time){
	unsigned int second,minute,hour,week,day,month,year;
	unsigned int century,add_century=0;
	int ret;
	unsigned char buf[7];
	ret = i2c_smbus_read_i2c_block_data(g_client_rtc, SD2068_REG_SECONDS,7, buf);
	if(ret<0)
	  return ret;
	//根据寄存器的定义,分别得到具体的数
	second = buf[0];
	minute = buf[1];
	hour   = buf[2];
	week   = buf[3];
	day    = buf[4];
	month  = buf[5];
	year   = buf[6];
	//write to rtc_time structure;
	time->tm_sec = (bcd2bin(second) > 59 ? 0 : bcd2bin(second));
	time->tm_min = (bcd2bin(minute) > 59 ? 0 : bcd2bin(minute));
	time->tm_hour = bcd2bin(hour);//24小时
	time->tm_wday = (bcd2bin(week))-1;
	time->tm_mday = bcd2bin(day);
	time->tm_mon = bcd2bin(month&0x7f) - 1;

	century = month &0x80;
	if(century)
		add_century = 100;

	time->tm_year = bcd2bin(year) + add_century+2000;

	printk("get time:%d-%02d-%02d %02d:%02d:%02d\n",time->tm_year,time->tm_mon,time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec);

	return rtc_valid_tm(time);
}

static int sd2068_rtc_set_time(struct device *dev, struct rtc_time *time){
	unsigned char buf[7];
	int ret;
	
	buf[0] = bin2bcd(time->tm_sec);
	buf[1] = bin2bcd(time->tm_min);
	buf[2] = bin2bcd(time->tm_hour);
	buf[3] = bin2bcd(time->tm_wday + 1);
	buf[4] = bin2bcd(time->tm_mday);
	buf[5] = bin2bcd(time->tm_mon + 1);
	if(time->tm_year >= 100){
		buf[5] |= 0x80;
		buf[6] = bin2bcd(time->tm_year - 100);
	}else{
		buf[6] = bin2bcd(time->tm_year);
	}
	ret = i2c_smbus_write_i2c_block_data(g_client_rtc, SD2068_REG_SECONDS, 7, buf);
	if(ret<0)
	  return ret;
	printk("set time:%02d-%02d-%02d %02d:%0xd:%02d\n",buf[6],buf[5],buf[4],
				buf[2],buf[1],buf[0]);
 
	return 0;
}

static struct rtc_class_ops sd2068_rtc_ops = {
	.read_time = sd2068_rtc_read_time,
	.set_time  = sd2068_rtc_set_time,
};

static int rtc_sd2068_probe(struct i2c_client *client,
			const struct i2c_device_id *id){
	g_client_rtc = client;

	rtc = devm_rtc_device_register(&client->dev,client->name,
				&sd2068_rtc_ops,
				THIS_MODULE);
	if(IS_ERR(rtc))
	  return -1;

	printk("bshui rtc_sd2068_probe--------\n");

	return 0;
}

static int rtc_sd2068_remove(struct i2c_client *client){

	printk("bshui rtc_sd2068_remove---\n");
	return 0;
}


static const struct i2c_device_id rtc_id[] = {
	{DRIVER_NAME, 0},
	{ }
};

static const struct of_device_id rtc_dt_ids[] = {
	{ .compatible = "sd2068",},
	{ }
};
MODULE_DEVICE_TABLE(of,rtc_dt_ids);

static struct i2c_driver rtc_sd2068_driver = {
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(rtc_dt_ids),
	},
	.probe    = rtc_sd2068_probe,
	.remove   = rtc_sd2068_remove,
	.id_table = rtc_id,
};

static int __init rtc_sd2068_init(void){
	return i2c_add_driver(&rtc_sd2068_driver);
}

static void __exit rtc_sd2068_exit(void){
	i2c_del_driver(&rtc_sd2068_driver);
}

module_init(rtc_sd2068_init);
module_exit(rtc_sd2068_exit);

MODULE_LICENSE("GPL");
