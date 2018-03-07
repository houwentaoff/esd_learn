/*************************************************************************
	> File Name: tca9535.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年11月02日 星期三 16时00分37秒
 ************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/err.h>

#define TCA9535_NAME "tca9535"

#define TCA9535_INPUT_REG0  0x00
#define TCA9535_INPUT_REG1  0x01
#define TCA9535_OUTPUT_REG0 0x02
#define TCA9535_OUTPUT_REG1 0x03
#define TCA9535_POLARITY_REG0 0x04
#define TCA9535_POLARITY_REG1 0x05
#define TCA9535_CONFIG_REG0   0x06
#define TCA9535_CONFIG_REG1   0x07


static const unsigned short normal_i2c[2] = {0x20, I2C_CLIENT_END};

static struct i2c_device_id tca9535_id[] = {
	{TCA9535_NAME, 0},
	{}
};

static int tca9535_read_reg(struct i2c_client *client, const u8 reg){
	s32 data = i2c_smbus_read_word_data(client, reg);
	return data;
}

static int tca9535_write_reg(struct i2c_client *client, const u8 reg,
			const u16 data){
	
	return i2c_smbus_write_word_data(client, reg, data);
}


static void tca9535_gpio_direction_input(struct i2c_client *client, u16 off){
	tca9535_write_reg(client, TCA9535_CONFIG_REG0, 0x1<<off);
}

static void tca9535_gpio_direction_output(struct i2c_client *client, u16 off, int val){
	//set output level
	if(val)
	  tca9535_write_reg(client, TCA9535_OUTPUT_REG0,0x1<<off);
	else
	  tca9535_write_reg(client, TCA9535_OUTPUT_REG0, ~(0x1<<off));

	//set direction
	tca9535_write_reg(client, TCA9535_CONFIG_REG0, ~(0x1<<off));
}

static int tca9535_gpio_get_value(struct i2c_client *client, u16 off){
	u16 reg_val;

	reg_val = tca9535_read_reg(client, TCA9535_INPUT_REG0);

	return (reg_val &(0x1 << off))?1:0;
}

static void tca9535_set_value(struct i2c_client *client, u16 off, int val){
	if(val)
	  tca9535_write_reg(client, TCA9535_OUTPUT_REG0, 0x1<<off);
	else
	  tca9535_write_reg(client, TCA9535_OUTPUT_REG0, ~(0x1<<off));
}
static void tca9535_init_default(struct i2c_client *client){
	tca9535_write_reg(client, TCA9535_OUTPUT_REG0, 0xffff);
	tca9535_write_reg(client, TCA9535_POLARITY_REG0, 0x0000);
	tca9535_write_reg(client, TCA9535_CONFIG_REG0, 0x0000);
}

static int tca9535_probe(struct i2c_client *client, const struct 
			i2c_device_id *id){
	tca9535_init_default(client);
	tca9535_gpio_direction_output(client, 0x3, 1);
	printk("bshui tca9535_probe--------\n");

	return 0;
}

static int tca9535_remove(struct i2c_client *client){

	return 0;
}

static int tca9535_detect(struct i2c_client *client, struct i2c_board_info *info){
	struct i2c_adapter *adapter = client->adapter;
	int twi_id = 1;
	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
	  return -ENODEV;

	if(twi_id == adapter->nr){
		strlcpy(info->type,TCA9535_NAME, I2C_NAME_SIZE);
		return 0;
	}else{
		return -ENODEV;
	}
	
}
static struct i2c_driver tca9535_driver ={
	.driver = {
		.name = TCA9535_NAME,
	},
	.probe = tca9535_probe,
	.remove = tca9535_remove,
	.id_table = tca9535_id,
	.address_list = normal_i2c,
	.detect = tca9535_detect,
};

static int __init tca9535_init(void){
	int ret;
	ret = i2c_add_driver(&tca9535_driver);

	return ret;
}

static void __exit tca9535_exit(void){
	i2c_del_driver(&tca9535_driver);
}

module_init(tca9535_init);
module_exit(tca9535_exit);
MODULE_LICENSE("GPL");
