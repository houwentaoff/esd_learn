/*************************************************************************
	> File Name: bmp280.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年06月02日 星期五 09时03分55秒
 ************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define BMP280_NAME "bmp280"
#define BMP280_REG_TEMP_XLSB  0xFC
#define BMP280_REG_TEMP_LSB   0xFB
#define BMP280_REG_TEMP_MSB   0xFA
#define BMP280_REG_PRESS_XLSB 0xF9
#define BMP280_REG_PRESS_LSB  0xF8
#define BMP280_REG_PRESS_MSB  0xF7
#define BMP280_REG_CONFIG     0xF5
#define BMP280_REG_CTRL_MEAS  0xF4
#define BMP280_REG_STATUS     0xF3
#define BMP280_REG_RESET      0xE0

#define MISC_IOC_MAGIC 'B'
#define GET_TEMP_DATA   _IO(MISC_IOC_MAGIC, 0x00)
#define GET_PRESS_DATA  _IO(MISC_IOC_MAGIC, 0x01)
#define CMD_MAXNR 2

struct bmp280_data {
	struct i2c_client *client;
	s32 t_fine;
	//compensation parameters
	u16 dig_t1;
	s16 dig_t2,dig_t3;
	u16 dig_p1;
	s16 dig_p2,dig_p3,dig_p4,dig_p5,dig_p6,dig_p7,dig_p8,dig_p9;
};

struct bmp280_data *g_data;

static const unsigned short normal_i2c[2] = {0x76, I2C_CLIENT_END};

//hPa/100
static u32 bmp280_read_press(void){
	u8 buf[3]={0};
	s32 adc_P;
	s32 var1,var2;
	u32 p;


	i2c_smbus_read_i2c_block_data(g_data->client, BMP280_REG_PRESS_MSB, 3, buf);

	adc_P = (buf[0]<<12)|(buf[1]<<4)|(buf[2]>>4);

	var1 = (((s32)g_data->t_fine)>>1)- (s32)64000;
	var2 = (((var1>>2)*(var1>>2))>>11)*((s32)g_data->dig_p6);
	var2 = var2+((var1*((s32)g_data->dig_p5))<<1);
	var2 = (var2>>2)+(((s32)g_data->dig_p4)<<16);
	var1 = (((g_data->dig_p3*(((var1>>2)*(var1>>2))>>13))>>3)+((((s32)g_data->dig_p2)*var1)>>1))>>18;
	var1 = ((((32768+var1))*((s32)g_data->dig_p1))>>15);

	if(var1 == 0){
		return 0;
	}
	p=(((s32)(((s32)1048576)-adc_P)-(var2>>12)))*3125;
	if(p<0x80000000){
		p=(p<<1)/((s32)var1);
	}else{
		p=(p/(u32)var1)*2;
	}

	var1 = (((s32)g_data->dig_p9)*((s32)(((p>>3)*(p>>3))>>13)))>>12;
	var2 = (((s32)(p>>2))*((s32)g_data->dig_p8))>>13;
	p = (u32)((s32)p+((var1+var2+g_data->dig_p7)>>4));

	return p;
}
//refer p46 DegC/100
static s32 bmp280_read_temp(void){
	u8 buf[3]={0};
	s32 var1,var2,adc_temp, T;
	
	i2c_smbus_read_i2c_block_data(g_data->client, BMP280_REG_TEMP_MSB,
				3, buf);

	adc_temp = (buf[0]<<12)|(buf[1]<<4)|(buf[2]>>4);

	var1 = ((((adc_temp >>3) - ((s32)g_data->dig_t1<<1)))*((s32)g_data->dig_t2))>>11;

	var2 = (((((adc_temp >>4) - ((s32)g_data->dig_t1))*((adc_temp>>4)-((s32)g_data->dig_t1)))>>12)*((s32)g_data->dig_t3))>>14;

	g_data->t_fine = var1+var2;

	T = ((g_data->t_fine*5 +128) >> 8);

	return T;
}

static void bmp280_chip_init(struct i2c_client *client){
	i2c_smbus_write_byte_data(client, BMP280_REG_CTRL_MEAS, 0x3F);
	i2c_smbus_write_byte_data(client, BMP280_REG_CONFIG, 0x0C);
}

static int bmp280_read_calibration_regs(struct i2c_client *client,
			 struct bmp280_data *data){
	s32 ret;
	u8 buf[24]={0};

	ret = i2c_smbus_read_i2c_block_data(client, 0x88, 24, buf);
	if(ret <0){
		printk("%s read i2c block fail\n", __func__);
		return -1;
	}

	data->dig_t1 = (buf[1]<<8) |buf[0];
	data->dig_t2 = (buf[3]<<8) |buf[2];
	data->dig_t3 = (buf[5]<<8) |buf[4];
	data->dig_p1 = (buf[7]<<8) |buf[6];
	data->dig_p2 = (buf[9]<<8) |buf[8];
	data->dig_p3 = (buf[11]<<8)|buf[10];
	data->dig_p4 = (buf[13]<<8)|buf[12];
	data->dig_p5 = (buf[15]<<8)|buf[14];
	data->dig_p6 = (buf[17]<<8)|buf[16];
	data->dig_p7 = (buf[19]<<8)|buf[18];
	data->dig_p8 = (buf[21]<<8)|buf[20];
	data->dig_p9 = (buf[23]<<8)|buf[22];
	return 0;
}

static long bmp280_misc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	
	void __user *data;
	s32 temp;
	u32 press;
	if(_IOC_TYPE(cmd) != MISC_IOC_MAGIC)
	  return -EFAULT;
	if(_IOC_NR(cmd)>CMD_MAXNR)
	  return -EFAULT;
	switch(cmd){
		case GET_TEMP_DATA:

			data = (void __user *)arg;
			if(data== NULL)
				return -EINVAL;

			temp = bmp280_read_temp();
			printk("temp from kernel is %d\n", temp);
			if(copy_to_user(data, &temp, sizeof(temp))){
			
				return -EFAULT;
			}

		break;
		case GET_PRESS_DATA:
			data = (void __user *)arg;
			if(data== NULL)
				return -EINVAL;
			press = bmp280_read_press();
			printk("press from kernel is %d\n", press);
			if(copy_to_user(data,&press, sizeof(press))){
				return -EFAULT;
			}

		break;
	}

	return 0;
}

static const struct file_operations bmp280_misc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = bmp280_misc_ioctl,
};

static struct miscdevice bmp280_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "bmp280",
	.fops  = &bmp280_misc_fops,
};

static int bmp280_probe(struct i2c_client *client, const struct i2c_device_id *id){

	int ret = 0;
	struct bmp280_data *data;
	data = kzalloc(sizeof(struct bmp280_data), GFP_KERNEL);
	if(!data){
		printk("kzalloc data fail %s\n",__func__);
		return -ENOMEM;
	}
	
	i2c_set_clientdata(client, data);
	data->client = client;
	ret = bmp280_read_calibration_regs(client,data);
	if(ret<0){
		printk("%s read calibration regs fail\n",__func__);
		return -1;
	}

	bmp280_chip_init(client);
	
	g_data = data;

	misc_register(&bmp280_misc_device);

	return 0;
}

static int bmp280_remove(struct i2c_client *client){
	misc_deregister(&bmp280_misc_device);	
	return 0;
}

static int bmp280_suspend(struct i2c_client *client, pm_message_t state){

	i2c_smbus_write_byte_data(client, BMP280_REG_CTRL_MEAS, 0x00);

	return 0;
}

static int bmp280_resume(struct i2c_client *client){
	
	bmp280_chip_init(client);
	return 0;
}
static struct i2c_device_id bmp280_id[] = {
	{BMP280_NAME, 0},
	{}
};

static struct i2c_driver bmp280_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = BMP280_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = bmp280_probe,
	.remove   = bmp280_remove,
	.suspend  = bmp280_suspend,
	.resume   = bmp280_resume,
	.id_table = bmp280_id,
	.address_list = normal_i2c,
};

static int bmp280_detect(struct i2c_client *client, struct i2c_board_info *info){
	struct i2c_adapter *adapter = client->adapter;
	int twi_id = 1;

	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
	  return -ENODEV;
	if(twi_id == adapter->nr){
		strlcpy(info->type, BMP280_NAME, I2C_NAME_SIZE);
		printk("bmp280 %s detect success\n", __func__);
		return 0;
	}else{
		return -ENODEV;
	}
}

static int __init bmp280_init(void){
	bmp280_driver.detect = bmp280_detect;
	i2c_add_driver(&bmp280_driver);

	return 0;
}

static void __exit bmp280_exit(void){
	i2c_del_driver(&bmp280_driver);
}

module_init(bmp280_init);
module_exit(bmp280_exit);
MODULE_LICENSE("GPL");
