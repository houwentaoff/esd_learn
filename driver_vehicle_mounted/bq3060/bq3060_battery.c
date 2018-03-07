/*************************************************************************
	> File Name: bq3060_battery.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2017年06月05日 星期一 19时12分40秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>

#define BQ3060_NAME "bq3060"

#define REG_TEMPERATURE 0x08
#define REG_VOLTAGE     0x09 //battery voltage
#define REG_CURRENT     0x0a //battery current
#define REG_RSOC        0x0d
#define REG_COUNTER     0x17
#define REG_CHGVOLTAGE  0x15
#define REG_CHGCURRENT  0x14
#define REG_DESIGNCAP   0x18 //8700mAh
#define REG_NAME        0X20

#define WORK_DELAY_TIME 1000
#define DELAY_USLEEP   200 //100us if error
#define DELAY_CMD  500 //500us
#define RETRY_TIME 100  //if error read retry 100 times
#define DC_IN GPIOD(24)
static int g_current=0;
static const unsigned short normal_i2c[2] = {0x0b, I2C_CLIENT_END};

struct bq3060_info {
	struct i2c_client *client;
	struct delayed_work work;
	struct power_supply batt;
	struct power_supply ac;
	int battery_present;
	int battery_current;
	int ac_online;
	int battery_rsoc;
};

static enum power_supply_property bq3060_battery_props[]= {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_ENERGY_FULL,
};

static enum power_supply_property bq3060_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
};

static unsigned int bq3060_ac_voltage(struct bq3060_info *di){
	udelay(DELAY_CMD);
#if 1
	unsigned int volt;
	int i;
	for(i=0; i<RETRY_TIME; i++){
	volt = i2c_smbus_read_word_data(di->client, REG_CHGVOLTAGE);
	if(volt >= 0){
		break;
	}
	printk("bshui %s volt=%d\n", __func__, volt);
	}
#endif
	udelay(DELAY_CMD);
	return volt;
}

static unsigned int bq3060_ac_current(struct bq3060_info *di){
	udelay(DELAY_CMD);
#if 1
	unsigned int curr;
	int i;
	for(i=0; i<RETRY_TIME;i++){
	curr = i2c_smbus_read_word_data(di->client, REG_CHGCURRENT);
	if(current >=0)
	  break;
	printk("bshui %s curr=%d\n", __func__, curr);
	}
#endif
	
	udelay(DELAY_CMD);
	return curr;
}

static int bq3060_ac_online(struct bq3060_info *di){
	int online = 0;
	online = gpio_get_value(DC_IN);

	di->ac_online = !online;

	return !online;
}

static int bq3060_ac_get_property(struct power_supply *psy,
			enum power_supply_property psp, 
			union power_supply_propval *val){

	struct bq3060_info *di = container_of((psy),
				struct bq3060_info, ac);

	switch(psp){
		case POWER_SUPPLY_PROP_ONLINE:
			val->intval = bq3060_ac_online(di);
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = bq3060_ac_voltage(di);
			break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			val->intval = bq3060_ac_current(di);
			break;
	}

	return 0;
}

static int bq3060_battery_current(struct bq3060_info *di){
	short int curr = 0;
	int i;
	for(i=0;i<RETRY_TIME;i++){
	curr  = i2c_smbus_read_word_data(di->client, REG_CURRENT);
	if(curr>=-32768 && curr<=32767)
	  break;
		udelay(DELAY_USLEEP);
	printk("bshui %s curr=%d\n",__func__, curr);
	}

	udelay(DELAY_CMD);
	return curr;

}

static int bq3060_battery_status(struct bq3060_info *di){
	short int status = 0;

	if(di->ac_online){
	
		status = POWER_SUPPLY_STATUS_CHARGING;
		if(di->battery_rsoc==100)
		  status = POWER_SUPPLY_STATUS_FULL;
	}else{
		status = POWER_SUPPLY_STATUS_DISCHARGING;
	}
	

	udelay(DELAY_CMD);
	return status;
}

static unsigned int bq3060_battery_rsoc(struct bq3060_info *di){
	unsigned char rsoc = 0;
	int i=0;
	for(i=0;i<RETRY_TIME;i++){
		dump_stack();
		rsoc = i2c_smbus_read_word_data(di->client, REG_RSOC);
		if(rsoc>=0 && rsoc<=100){
			break;
		}

		printk("bshui %s rsoc=%d\n", __func__, rsoc);
		udelay(DELAY_USLEEP);
	}
	if(rsoc==100){
		if(g_current<=0)
		  rsoc=100;
		else
		  rsoc=99;
	}
	di->battery_rsoc = rsoc;
	udelay(DELAY_CMD);
	return rsoc;
}

static unsigned int bq3060_battery_voltage(struct bq3060_info *di){
	unsigned int volt = 0;
	int i= 0;
	for(i=0; i<RETRY_TIME;i++){
	volt = i2c_smbus_read_word_data(di->client, REG_VOLTAGE);
	if(volt >=0)
	  break;
	udelay(DELAY_USLEEP);
	printk("bshui %s voltage=%d\n", __func__, volt);
	}

	udelay(DELAY_CMD);
	return volt;

}

static unsigned int bq3060_battery_temp(struct bq3060_info *di){
	unsigned int temp=0;
#if 0
	int i;
	for(i=0;i<RETRY_TIME;i++){
	temp = i2c_smbus_read_word_data(di->client, REG_TEMPERATURE);
	if(temp>=0&&temp<=65535)
	  break;
	printk("bshui %s temp=%d\n",__func__, temp);
	udelay(DELAY_USLEEP);
	}
	temp = temp/10 -273;

	return temp;
#endif 
	return 28;
}

static unsigned int bq3060_battery_counter(struct bq3060_info *di){
#if 0
	unsigned int counter = 0;
	int i;
	for(i=0;i<RETRY_TIME;i++){
	counter = i2c_smbus_read_word_data(di->client, REG_COUNTER);
	if(counter >=0 )
	  break;
	udelay(DELAY_USLEEP);
	printk("bshui %s counter=%d\n",__func__, counter);
	}


	return counter;
#endif
	return 0;
}

static unsigned int bq3060_battery_energy_full(struct bq3060_info *di){
	unsigned int energy=0;
	int i;
	if(energy!=8100){
	for(i=0;i<RETRY_TIME;i++){
	energy = i2c_smbus_read_word_data(di->client, REG_DESIGNCAP);
	if(energy >=0 && energy<=65535)
	  break;
	udelay(DELAY_USLEEP);
	printk("bshui %s energy=%d\n",__func__, energy);
	}
	}

	udelay(DELAY_CMD);
	return energy;
}

static int bq3060_battery_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val){
	struct bq3060_info *di = container_of((psy),
				struct bq3060_info, batt);

	switch(psp){
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = bq3060_battery_status(di);
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = 1;//di->battery_present;
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			val->intval = bq3060_battery_rsoc(di);
			 break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			 val->intval = bq3060_battery_voltage(di);
			 break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			 val->intval = bq3060_battery_current(di);
			 g_current = val->intval;
			 break;
		case POWER_SUPPLY_PROP_TEMP:
			 val->intval = bq3060_battery_temp(di);
			 break;
		case POWER_SUPPLY_PROP_CYCLE_COUNT:
			 val->intval = bq3060_battery_counter(di);
			 break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			 val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
			 break;
		case POWER_SUPPLY_PROP_ENERGY_FULL:
			 val->intval = bq3060_battery_energy_full(di);
			 break;
	}


	return 0;
}

static void bq3060_battery_work(struct work_struct *work){
	struct bq3060_info *di = container_of(work, struct bq3060_info,
				work);
	power_supply_changed(&di->ac);
//	power_supply_changed(&di->batt);
	schedule_delayed_work(&di->work, WORK_DELAY_TIME);

}

static int bq3060_probe(struct i2c_client *client, const struct i2c_device_id *id){
	struct bq3060_info *di;
	int ret = 0;
	unsigned int name=0;
	int i=0;

	di = kzalloc(sizeof(struct bq3060_info), GFP_KERNEL);
	if(!di)
	  return -ENOMEM;

	di->client = client;

	i2c_set_clientdata(client, di);
	//powersupply_init
	di->batt.name = "bq3060_battery";
	di->batt.type = POWER_SUPPLY_TYPE_BATTERY;
	di->batt.properties = bq3060_battery_props;
	di->batt.num_properties = ARRAY_SIZE(bq3060_battery_props);
	di->batt.get_property = bq3060_battery_get_property;

	di->ac.name = "bq3060_ac";
	di->ac.type = POWER_SUPPLY_TYPE_MAINS;
	di->ac.properties = bq3060_ac_props;
	di->ac.num_properties = ARRAY_SIZE(bq3060_ac_props);
	di->ac.get_property = bq3060_ac_get_property;
#if 0
	name = i2c_smbus_read_word_data(di->client, REG_NAME);
	if(name != 18948)
	  return 0;
#endif	


	ret = power_supply_register(&client->dev, &di->batt);
	if(ret){
		return -1;
	}

	ret = power_supply_register(&client->dev, &di->ac);
	if(ret)
	  return -1;

	gpio_request_one(DC_IN, GPIOF_IN, "dc_in");

	INIT_DELAYED_WORK(&di->work, bq3060_battery_work);
	schedule_delayed_work(&di->work, WORK_DELAY_TIME);



	return 0;
}

static int bq3060_remove(struct i2c_client *client){
	struct bq3060_info *di = i2c_get_clientdata(client);
	
	cancel_delayed_work_sync(&di->work);
	power_supply_unregister(&di->batt);
	power_supply_unregister(&di->ac);
	kfree(di);

	return 0;
}

static int bq3060_suspend(struct i2c_client *client, pm_message_t state){
	struct bq3060_info *di = i2c_get_clientdata(client);

	cancel_delayed_work_sync(&di->work);
	return 0;
}

static int bq3060_resume(struct i2c_client *client){
	struct bq3060_info *di = i2c_get_clientdata(client);
	schedule_delayed_work(&di->work, 0);
	return 0;
}

static struct i2c_device_id bq3060_id[] = {
	{BQ3060_NAME, 0},
	{}
};

static struct i2c_driver bq3060_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = BQ3060_NAME,
		.owner = THIS_MODULE,
	},
	.probe   = bq3060_probe,
	.remove  = bq3060_remove,
	.suspend = bq3060_suspend,
	.resume  = bq3060_resume,
	.id_table = bq3060_id,
	.address_list = normal_i2c,
};


static int bq3060_detect(struct i2c_client *client, struct i2c_board_info *info){
	struct i2c_adapter *adapter = client->adapter;
	int twi_id = 1;

	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
	  return -ENODEV;
	if(twi_id == adapter->nr){
		strlcpy(info->type, BQ3060_NAME, I2C_NAME_SIZE);
		return 0;
	}else
	  return -ENODEV;
}

static int __init bq3060_init(void){
	bq3060_driver.detect = bq3060_detect;
	i2c_add_driver(&bq3060_driver);

	return 0;
}

static void __exit bq3060_exit(void){
	i2c_del_driver(&bq3060_driver);
}


module_init(bq3060_init);
module_exit(bq3060_exit);
MODULE_LICENSE("GPL");
