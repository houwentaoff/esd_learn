/*************************************************************************
	> File Name: i2c-sketch.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年10月23日 星期一 10时31分57秒
 ************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#define DRIVER_NAME "AAA_BBB"
static struct i2c_client *g_client_AAA;

static int AAA_write_reg(unsigned int reg, unsigned char value){
	int ret;
	ret = i2c_smbus_write_byte_data(g_client_AAA, reg, value);
	if(ret < 0){
		printk("bshui AAA_write_reg reg=0x%x failed\n", reg);
	}
	return ret;
}

static unsigned char AAA_read_reg(unsigned int reg){
	return i2c_smbus_read_byte_data(g_client_AAA, reg);
}

static int AAA_BBB_probe(struct i2c_client *client,
			const struct i2c_device_id *id){
	printk("bshui AAA_BBB_probe--------\n");

	return 0;
}

static int AAA_BBB_remove(struct i2c_client *client){

	printk("bshui AAA_BBB_remove---\n");
	return 0;
}


static const struct i2c_device_id AAA_id[] = {
	{DRIVER_NAME, 0},
	{ }
};

static const struct of_device_id AAA_dt_ids[] = {
	{ .compatible = "ti,bq27541-g1",},
	{ }
};
MODULE_DEVICE_TABLE(of,AAA_dt_ids);

static struct i2c_driver AAA_BBB_driver = {
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(AAA_dt_ids),
	},
	.probe    = AAA_BBB_probe,
	.remove   = AAA_BBB_remove,
	.id_table = AAA_id,
};

static int __init AAA_BBB_init(void){
	return i2c_add_driver(&AAA_BBB_driver);
}

static void __exit AAA_BBB_exit(void){
	i2c_del_driver(&AAA_BBB_driver);
}

module_init(AAA_BBB_init);
module_exit(AAA_BBB_exit);

MODULE_LICENSE("GPL");
