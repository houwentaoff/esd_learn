/*************************************************************************
	> File Name: bq27514_battery.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年09月08日 星期四 14时29分58秒
 ************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>

#define BQ27541_REG_CNTL  0x00  
#define BQ27541_REG_TEMP  0x06
#define BQ27541_REG_VOLT  0x08
#define BQ27541_REG_FLAGS 0x0A
#define BQ27541_REG_RM    0x10
#define BQ27541_REG_FCC   0x12 //Full Charge Capacity
#define BQ27541_REG_AI    0x14 //Average Current
#define BQ27541_REG_AP    0x24 //Average power 
#define BQ27541_REG_SOC   0x2c //Relative state of Charge
#define WORK_DELAY_TIME  1000

static const unsigned short normal_i2c[2] = {0x55, I2C_CLIENT_END};

struct bq27541_device_info{
	struct delayed_work work;
	struct device      *dev;
	struct i2c_client  *client;
	struct power_supply bat;
};

static int bq27541_reg_read(struct i2c_client *client, const u8 reg){
	s32 data = i2c_smbus_read_word_data(client, reg);

	return data;
}

static int bq27541_reg_write(struct i2c_client *client, const u8 reg,
			const u16 data){
	return i2c_smbus_write_word_data(client,reg,data);
}

static int bq27541_battery_voltage(struct bq27541_device_info *di){
	int volt = 0;

	volt = bq27541_reg_read(di->client, BQ27541_REG_VOLT);

	printk("battery voltage=[%d]mV\n", volt);

	return volt*1000;
}

static int bq27541_battery_current(struct bq27541_device_info *di){
	int curr = 0;

	curr = bq27541_reg_read(di->client, BQ27541_REG_AI);
	if(curr > 32768)
	  curr = 65535-curr;

//	printk("battery current=[%d]mA\n", curr);

	return curr;
}

static int bq27541_battery_rsoc(struct bq27541_device_info *di){
	int rsoc = 0;
	int i=0;
	for(i=0; i<10; i++){
	rsoc = bq27541_reg_read(di->client, BQ27541_REG_SOC);
	if((rsoc<=100)&&(rsoc >= 0))
	  break;
	}
	printk("battery the battery level=[%d]%\n", rsoc);
	return rsoc;
}

static int bq27541_battery_status(struct bq27541_device_info *di){
	int status= 0;

	status = bq27541_reg_read(di->client, BQ27541_REG_FLAGS);
	if(status < 0){
		printk("battery read flasg error\n");
		return 0;
	}

	status = (status<<0)&0x1;
	if(!status){
		printk("battery charging\n");
		status = POWER_SUPPLY_STATUS_CHARGING;
	}else{
		printk("battery discharging\n");
		status = POWER_SUPPLY_STATUS_DISCHARGING;
	}
	
	return status;
}

static int bq27541_battery_temp(struct bq27541_device_info *di){
	int temp = 0;

	temp = bq27541_reg_read(di->client, BQ27541_REG_TEMP);

	if(temp){
		printk("battery temp error\n");
	}
	printk("battery the temp = %d\n", temp);

	temp = temp/10 -273;
	
	printk("battery temp=[%d]C\n", temp);
	return temp;
}
static enum power_supply_property bq27541_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,

};

static int bq27541_battery_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val){
		struct bq27541_device_info *di = container_of((psy),
						struct bq27541_device_info, bat);
		switch(psp){
			case POWER_SUPPLY_PROP_STATUS:
				val->intval = bq27541_battery_status(di);
			break;
			case POWER_SUPPLY_PROP_VOLTAGE_NOW:
				val->intval = bq27541_battery_voltage(di);
			break;
			case POWER_SUPPLY_PROP_CURRENT_NOW:
				val->intval = bq27541_battery_current(di);
			break;
			case POWER_SUPPLY_PROP_CAPACITY:
				val->intval = bq27541_battery_rsoc(di);
			break;
			case POWER_SUPPLY_PROP_TEMP:
				val->intval = bq27541_battery_temp(di);
			break;
			default:
				return -EINVAL;
		}


	return 0;
}

static void bq27541_battery_work(struct work_struct *work){
	struct bq27541_device_info *chip = container_of(work,
				struct bq27541_device_info,work);
	struct bq27541_device_info *di = i2c_get_clientdata(chip->client);


	power_supply_changed(&di->bat);

	schedule_delayed_work(&chip->work, WORK_DELAY_TIME);

}


static int bq27541_battery_probe(struct i2c_client *client,
			const struct i2c_device_id *id){
	char *name = "bq27541_battery";
	int ret = 0;

	struct bq27541_device_info *di;

	di = kzalloc(sizeof(struct bq27541_device_info), GFP_KERNEL);
	if(!di){
		printk("battery failed to allocate device info data\n");

		return -ENOMEM;
	}

	i2c_set_clientdata(client, di);

	di->dev = &client->dev;
	di->client = client;
	
	//powersupply init
	di->bat.name = name;
	di->bat.type = POWER_SUPPLY_TYPE_BATTERY;
	di->bat.properties = bq27541_battery_props;
	di->bat.num_properties = ARRAY_SIZE(bq27541_battery_props);
	di->bat.get_property = bq27541_battery_get_property;

	ret = power_supply_register(&client->dev, &di->bat);
	if(ret){
		printk("battery failed to register power_supply bq27541\n");
		return -1;
	}

	
	INIT_DELAYED_WORK(&di->work, bq27541_battery_work);
	schedule_delayed_work(&di->work, WORK_DELAY_TIME);
	return 0;
}

static int bq27541_battery_remove(struct i2c_client *client){
	
	struct bq27541_device_info *di = i2c_get_clientdata(client);
	
	cancel_delayed_work_sync(&di->work);

	power_supply_unregister(&di->bat);
	kfree(di);

	return 0;
}

static int bq27541_battery_suspend(struct i2c_client *client, pm_message_t state){
	struct bq27541_device_info *di = i2c_get_clientdata(client);

	cancel_delayed_work_sync(&di->work);

	return 0;
}

static int bq27541_battery_resume(struct i2c_client *client){
	struct bq27541_device_info *di = i2c_get_clientdata(client);
	
	schedule_delayed_work(&di->work, 0);
	return 0;
}

static struct i2c_device_id bq27541_id[] = {
	{"bq27541_battery", 0},
	{}
};

static struct i2c_driver bq27541_battery_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "bq27541_battery",
		.owner = THIS_MODULE,
	},
	.probe   = bq27541_battery_probe,
	.remove  = bq27541_battery_remove,
	.suspend = bq27541_battery_suspend,
	.resume  = bq27541_battery_resume,
	.id_table = bq27541_id,
	.address_list = normal_i2c,
};

static int battery_detect(struct i2c_client *client, struct i2c_board_info *info){
	struct i2c_adapter *adapter = client->adapter;
	int ret = 0;
	int twi_id = 1;
	
	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
	  return -ENODEV;
	
	if(twi_id == adapter->nr){
	//	printk("battery %s addr:0x%x\n", __func__,client->addr);
		
		strlcpy(info->type, "bq27541_battery", I2C_NAME_SIZE);

		return 0;
	}else{
		printk("battery %s detect fail\n", __func__);
		return -ENODEV;
	}


}

static int __init bq27541_battery_init(void){
	int ret = -1;
	

	bq27541_battery_driver.detect = battery_detect;

	ret = i2c_add_driver(&bq27541_battery_driver);
	if(ret){
		printk("battery unable to register bq27541 driver\n");
	}else{
		//printk("battery success register bq27541 driver\n");
	}

}

static void __exit bq27541_battery_exit(void){
	
	i2c_del_driver(&bq27541_battery_driver);
}

late_initcall(bq27541_battery_init);
module_exit(bq27541_battery_exit);

MODULE_LICENSE("GPL");



