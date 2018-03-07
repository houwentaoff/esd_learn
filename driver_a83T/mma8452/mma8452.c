/*************************************************************************
	> File Name: mma8452.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年10月17日 星期一 16时02分08秒
 ************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/input-polldev.h>
#include <linux/device.h>
#include <linux/init-input.h>
#include <linux/types.h>
#include <linux/mutex.h>

#define MMA8452_DRV_NAME "FreescaleAccelerometer"

static const unsigned short normal_i2c[2] = {0x1c, I2C_CLIENT_END};
static const struct i2c_device_id mma8452_id[] = {
	{MMA8452_DRV_NAME, 0},
	{}
};

enum {
	MODE_2G = 0,
	MODE_4G,
	MODE_8G,
};

enum {
	MMA_STANDBY = 0,
	MMA_ACTIVED,
};

struct mma8452_info{
	u8 mode;
	u8 ctl_reg1;
	struct i2c_client *client;
	struct input_polled_dev *idev;
	struct device *hwmon_dev;

};
static int position = 7;

static DEFINE_MUTEX(mma8452_lock);
static struct i2c_client *mma8452_i2c_client;
static int enable_id = 0;

#define DEFAULT_SENSTIVE_MODE MODE_2G
#define POLL_INTERVAL_MAX 500
#define POLL_INTERVAL_MIN 20
#define POLL_INTERVAL	  100
#define INPUT_FUZZ		32
#define INPUT_FLAT      32
#define MMA8452_BUF_SIZE 6

#define MMA8452_STATUS			  0x00
#define MMA8452_OUT_X_MSB         0x01
#define MMA8452_OUT_X_LSB         0x02
#define MMA8452_OUT_Y_MSB         0x03
#define MMA8452_OUT_Y_LSB         0x04
#define MMA8452_OUT_Z_MSB         0x05
#define MMA8452_OUT_Z_LSB         0x06

#define MMA8452_CTRL_REG1		  0x2A
#define MMA8452_XYZ_DATA_CFG	  0x0E
#define MMA8452_STATUS_ZYXDR	  0x08

static int init_mma8452_chip(struct mma8452_info *priv){
	int result;
	//put sensor into standby mode by clearing the active bit
	priv->ctl_reg1 = 0x20;
	result = i2c_smbus_write_byte_data(priv->client,
				MMA8452_CTRL_REG1, priv->ctl_reg1);
	if(result < 0)
	  return result;
	//Write the 2g dynamic range value
	priv->mode = DEFAULT_SENSTIVE_MODE;
	result = i2c_smbus_write_byte_data(priv->client, MMA8452_XYZ_DATA_CFG,
				priv->mode);
	if(result < 0)
	  return result;
	priv->ctl_reg1 |= 0x01;
	i2c_smbus_write_byte_data(priv->client, MMA8452_CTRL_REG1, priv->ctl_reg1);

	mdelay(100);
	return 0;
}

static int mma8452_read_data(struct mma8452_info *priv, short *x, short *y, short *z){
		u8 tmp_data[MMA8452_BUF_SIZE];
		int ret;
		//Read 14-bit XYZ results using 6 byte
		ret = i2c_smbus_read_i2c_block_data(priv->client, MMA8452_OUT_X_MSB,MMA8452_BUF_SIZE, tmp_data);
		if(ret < MMA8452_BUF_SIZE){
			printk("mma8452 i2c block read failed\n");
			return -EIO;
		}
		//Concatenate the MSB and LSB
		*x = ((tmp_data[0]<<8)&0xff00) | tmp_data[1];
		*y = ((tmp_data[2]<<8)&0xff00) | tmp_data[3];
		*z = ((tmp_data[4]<<8)&0xff00) | tmp_data[5];

		*x = (short)(*x)>>4;
		*y = (short)(*y)>>4;
		*z = (short)(*z)>>4;
	return 0;
}

static int ACCHAL[8][3][3] = {
	{{0,-1,0},{1,0,0},{0,0,1}},
	{{-1,0,0},{0,-1,0},{0,0,1}},
	{{0,1,0},{-1,0,0},{0,0,1}},
	{{1,0,0},{0,1,0},{0,0,1}},

	{{0,-1,0},{-1,0,0},{0,0,-1}},
	{{-1,0,0},{0,1,0},{0,0,-1}},
	{{0,1,0},{1,0,0},{0,0,-1}},
	{{1,0,0},{0,-1,0},{0,0,-1}},
};
static int mma8452_adjust_position(short *x, short *y, short *z){
	short rawdata[3],data[3];
	int i,j;
	if(position <0 || position > 7)
	  position = 0;

	rawdata[0] = *x;
	rawdata[1] = *y;
	rawdata[2] = *z;
	for(i=0; i<3;i++){
		data[i] = 0;
		for(j=0; j<3; j++)
		  data[i] += rawdata[j]*ACCHAL[position][i][j];
	}
	*x = data[0];
	*y = data[1];
	*z = data[2];

	return 0;
}
static void mma8452_dev_poll(struct input_polled_dev *dev){
	struct mma8452_info *priv = dev->private;
	short x,y,z;
	int ret;
	
	mutex_lock(&mma8452_lock);
	  //read status register
	ret = i2c_smbus_read_byte_data(priv->client, MMA8452_STATUS);
	//Check ZYXDR status bit for data available
	if(!(ret & MMA8452_STATUS_ZYXDR)){
		//Data not ready
		//printk("mma8452 data not ready wait for new data\n");
		goto out;
	}
	if(enable_id){
		//Read XYZ data
	if(mma8452_read_data(priv, &x, &y, &z)!=0){
		printk("mma8452 no data\n");	
	  goto out;
	}
		mma8452_adjust_position(&x, &y, &z);

		//printk("mma8452 poll data x=0x%hx,y=0x%hx, z=0x%hx \n", x,y,z);
		//Report XYZ data
		input_report_abs(priv->idev->input, ABS_X, x);
		input_report_abs(priv->idev->input, ABS_Y, y);
		input_report_abs(priv->idev->input, ABS_Z, z);
		input_sync(priv->idev->input);
	}
		
out:
	mutex_unlock(&mma8452_lock);

}
static ssize_t mma8452_enable_store(struct device *dev,
			struct device_attribute *attr, const char *buf,
			size_t count){
	unsigned long data;
	int error;
	u8 ctl_reg1;

//	error = strict_strtoul(buf, 10, &data);
	data = simple_strtoul(buf, NULL, 10);
	data = (data > 0) ? 1: 0;

	if(data){
		enable_id = 1;
		ctl_reg1 = i2c_smbus_read_byte_data(mma8452_i2c_client, MMA8452_CTRL_REG1);
		ctl_reg1 |=0x1;
		error = i2c_smbus_write_byte_data(mma8452_i2c_client, MMA8452_CTRL_REG1, ctl_reg1);
	}else{
		enable_id = 0;

		ctl_reg1 = i2c_smbus_read_byte_data(mma8452_i2c_client, MMA8452_CTRL_REG1);
		error = i2c_smbus_write_byte_data(mma8452_i2c_client, MMA8452_CTRL_REG1, ctl_reg1 & 0xFE);
	}
	return error;
}

static ssize_t mma8452_enable_show(struct device *dev,
			struct device_attribute *attr, char *buf){
	int enable;
	u8 val;
	mutex_lock(&mma8452_lock);
	val = i2c_smbus_read_byte_data(mma8452_i2c_client, MMA8452_CTRL_REG1);
	if((val &0x01))
	  enable = 1;
	else
	  enable = 0;
	mutex_unlock(&mma8452_lock);

	return sprintf(buf, "%d\n", enable);
}
static ssize_t mma8452_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){
	unsigned long data;
	int error;
	error = strict_strtoul(buf, 10, &data);
	if(error)
	  return error;
	if(data > POLL_INTERVAL_MAX)
		data = POLL_INTERVAL_MAX;
	printk("mma8452 delay store\n");
	return count;
}

static ssize_t mma8452_position_show(struct device *dev,
			struct device_attribute *attr, char *buf){
	return sprintf(buf, "%d\n", position);
}

static ssize_t mma8452_position_store(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count){
	position = simple_strtoul(buf, NULL, 10);

	return count;
}

static DEVICE_ATTR(enable, 0666, mma8452_enable_show, mma8452_enable_store);
static DEVICE_ATTR(delay, 0666, NULL, mma8452_delay_store);
static DEVICE_ATTR(position,0666, mma8452_position_show,mma8452_position_store);
static struct attribute *mma8452_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_delay.attr,
	&dev_attr_position.attr,
	NULL
};

static struct attribute_group mma8452_attr_group = {
	.attrs = mma8452_attributes,
};
static int mma8452_probe(struct i2c_client *client, const struct 
			i2c_device_id *id){
	int ret = -1;
	struct input_dev *input_idev;
	struct mma8452_info *priv;
	
	priv = kzalloc(sizeof(struct mma8452_info), GFP_KERNEL);
	if(!priv){
		printk("failed to alloc driver mma8452 info\n");
		return ret;
	}

	priv->client = client;
	mma8452_i2c_client = client;

	priv->mode = DEFAULT_SENSTIVE_MODE;
	//initialize the mma8452 chip
	ret = init_mma8452_chip(priv);
	if(ret){
		printk("mma8452 init chip fail\n");
		kfree(priv);
		return ret;
	}	

	priv->hwmon_dev = hwmon_device_register(&client->dev);
	if(!priv->hwmon_dev){
		ret = -ENOMEM;
		printk("mma8452 error register hwmon device\n");
		kfree(priv);
		return ret;
	}

	//input poll device register
	priv->idev = input_allocate_polled_device();
	if(!priv->idev){
		ret = -ENOMEM;
		printk("mma8452 poll device failed\n");
		hwmon_device_unregister(&client->dev);
		return ret;
	}
	
	priv->idev->private = priv;
	priv->idev->poll = mma8452_dev_poll;
	priv->idev->poll_interval = POLL_INTERVAL;
	priv->idev->poll_interval_max = POLL_INTERVAL_MAX;

	input_idev = priv->idev->input;
	input_idev->name = MMA8452_DRV_NAME;
	input_idev->id.bustype = BUS_I2C;
	input_idev->evbit[0] = BIT_MASK(EV_ABS);

	input_set_abs_params(input_idev, ABS_X, -8192, 8191, INPUT_FUZZ, INPUT_FLAT);
	input_set_abs_params(input_idev, ABS_Y, -8192, 8191, INPUT_FUZZ, INPUT_FLAT);
	input_set_abs_params(input_idev, ABS_Z, -8192, 8191, INPUT_FUZZ, INPUT_FLAT);

	ret = input_register_polled_device(priv->idev);
	if(ret){
		printk("mma8452 register poll device failed\n");
		input_free_polled_device(priv->idev);
	}

	ret = sysfs_create_group(&priv->idev->input->dev.kobj, &mma8452_attr_group);
	if(ret){
		printk("mma8452 create device file failed\n");
		ret = -EINVAL;
		input_free_polled_device(priv->idev);
	}
	i2c_set_clientdata(client, priv);
	printk("bshui mma8452 probe over\n");

	return 0;
}

static int mma8452_stop_chip(struct i2c_client *client){
	struct mma8452_info *priv = i2c_get_clientdata(client);
	int ret;
	enable_id = 0;
	priv->ctl_reg1 = i2c_smbus_read_byte_data(client,MMA8452_CTRL_REG1);
	ret = i2c_smbus_write_byte_data(client, MMA8452_CTRL_REG1,
				priv->ctl_reg1&0xFE);
	return ret;
}

static int mma8452_remove(struct i2c_client *client){
	int ret;
	struct mma8452_info *priv = i2c_get_clientdata(client);
	ret = mma8452_stop_chip(client);
	input_free_polled_device(priv->idev);
	hwmon_device_unregister(priv->hwmon_dev);

	return ret;
}

static int mma8452_detect(struct i2c_client *client, struct i2c_board_info *info){
	struct i2c_adapter *adapter = client->adapter;
	int twi_id = 1;
	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
	  return -ENODEV;

	if(twi_id == adapter->nr){
		printk("mma8452 detect success addr=0x%x\n", client->addr);
		strlcpy(info->type, MMA8452_DRV_NAME, I2C_NAME_SIZE);
		return 0;
	}else{
		//printk("mma8452 detect fail\n");
		return -ENODEV;
	}
	
}

static int mma8452_suspend(struct i2c_client *client, pm_message_t msg){
	return mma8452_stop_chip(client);
}

static int mma8452_resume(struct i2c_client *client){
	int ctl_reg1;
	ctl_reg1 = i2c_smbus_read_byte_data(mma8452_i2c_client, MMA8452_CTRL_REG1);
	ctl_reg1 |= 0x01;
	i2c_smbus_write_byte_data(client, MMA8452_CTRL_REG1, ctl_reg1);
	return 0;
}
static struct i2c_driver mma8452_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = MMA8452_DRV_NAME,
		.owner = THIS_MODULE,
	},
	.probe  = mma8452_probe,
	.remove = mma8452_remove,
	.id_table = mma8452_id,
	.detect = mma8452_detect,
	.address_list = normal_i2c,
	.suspend = mma8452_suspend,
	.resume  = mma8452_resume,
};

static int __init mma8452_init(void){
	int ret=0;

	ret = i2c_add_driver(&mma8452_driver);
	if(ret < 0){
		printk("mma8452 i2c driver failed\n");
		return -ENODEV;
	}

	return ret;
}

static void __exit mma8452_exit(void){
	i2c_del_driver(&mma8452_driver);
}
module_init(mma8452_init);
module_exit(mma8452_exit);

MODULE_LICENSE("GPL");

