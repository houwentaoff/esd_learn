/*************************************************************************
	> File Name: bq27541-battery.c
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
#include <linux/workqueue.h>
#include <linux/power_supply.h>
#include <linux/device.h>

#define DRIVER_NAME "bq27541_battery"
#define WORK_DELAY_TIME 1000

static struct i2c_client *g_client_bq27541;

struct bq27541_device_info {
	struct delayed_work work;
	struct device *dev;
	struct i2c_client *client;
	struct power_supply *battery;
	struct power_supply_desc battery_desc;
	struct power_supply *ac;
	struct power_supply_desc ac_desc;
};

struct bq27541_device_info *g_di;


static int bq27541_read_reg(unsigned int reg){
	return i2c_smbus_read_word_data(g_client_bq27541, reg);
}


static void bq27541_battery_work(struct work_struct *work){
	power_supply_changed(g_di->ac);
	power_supply_changed(g_di->battery);
	schedule_delayed_work(&g_di->work, WORK_DELAY_TIME);
}

static enum power_supply_property bq27541_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property bq27541_battery_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TEMP,

};

static int bq27541_battery_status(void){
	int status = 0;
	status = bq27541_read_reg(0x0a);//REG_FLAGS
	if(status < 0){
		printk("battery read flags error\n");
		return 0;
	}
	status = (status<<0)&0x01;
	printk("status=0x%x\n", status);
	if(status==1){
		status = POWER_SUPPLY_STATUS_DISCHARGING;
	}else if(status==0){
		status = POWER_SUPPLY_STATUS_CHARGING;
	}

	return status;
}

static int bq27541_battery_voltage(void){
	int volt = 0;
	volt = bq27541_read_reg(0x08);//REG_Voltage
//	printk("battery voltage=%d mV\n", volt);
	return volt;
}

static int bq27541_battery_current(void){
	int curr = 0;
	curr = bq27541_read_reg(0x14);//AverageCurrent
//	printk("battery average current=%d mA\n", curr);
	return curr;
}

static int bq27541_battery_rsoc(void){
	int rsoc = 0;
	rsoc = bq27541_read_reg(0x2c);
//	printk("battery rsoc=%d\n", rsoc);
	if(rsoc > 100)
	  rsoc = 100;
	return rsoc;
}

static int bq27541_battery_temp(void){
	int temp = 0;
	temp = bq27541_read_reg(0x06);

	temp = temp/10 - 273;

//	printk("battery temp = %d C\n", temp);
	return temp;
}

static int bq27541_ac_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val){

	int online=0;

	switch(psp){
		case POWER_SUPPLY_PROP_ONLINE:
			online= bq27541_battery_status();
			if(online==1)
			  val->intval = 1;
			else
			  val->intval = 0;
			break;
		default:
			return -EINVAL;
	}
}

static int bq27541_battery_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val){
	switch(psp){
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = 1;
			break;
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = bq27541_battery_status();
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = bq27541_battery_voltage();
			break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			val->intval = bq27541_battery_current();
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			val->intval = bq27541_battery_rsoc();
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
			break;
		case POWER_SUPPLY_PROP_TEMP:
			val->intval = bq27541_battery_temp();
			break;
		default:
			return -EINVAL;
	}

	
	return 0;
}

static const struct power_supply_desc bq27541_ac_desc = {

	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = bq27541_ac_props,
	.num_properties = ARRAY_SIZE(bq27541_ac_props),
	.get_property = bq27541_ac_get_property,
};
static const struct power_supply_desc bq27541_bat_desc = {

	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = bq27541_battery_props,
	.num_properties = ARRAY_SIZE(bq27541_battery_props),
	.get_property = bq27541_battery_get_property,
};

static int bq27541_battery_probe(struct i2c_client *client,
			const struct i2c_device_id *id){
	struct bq27541_device_info *di;
	struct power_supply_config battery_psy_cfg = {};

	di = devm_kzalloc(&client->dev,sizeof(*di), GFP_KERNEL);
	if(!di){
		printk("failed to allocate device info data\n");
		return -1;
	}

	//powersupply init 
	di->dev = &client->dev;
	di->client = client;
	g_client_bq27541 = client;
	battery_psy_cfg.drv_data = di;

	di->battery = power_supply_register(di->dev,&bq27541_bat_desc,
				&battery_psy_cfg);
	
	di->ac = power_supply_register(di->dev, &bq27541_ac_desc,
				&battery_psy_cfg);

	INIT_DELAYED_WORK(&di->work, bq27541_battery_work);
	schedule_delayed_work(&di->work, WORK_DELAY_TIME);

	g_di = di;

	return 0;
}

static int bq27541_battery_remove(struct i2c_client *client){

	cancel_delayed_work_sync(&g_di->work);
	power_supply_unregister(g_di->ac);
	power_supply_unregister(g_di->battery);
	return 0;
}


static const struct i2c_device_id bq27541_id[] = {
	{DRIVER_NAME, 0},
	{ }
};

static const struct of_device_id bq27541_dt_ids[] = {
	{ .compatible = "ti,bq27541-g1",},
	{ }
};
MODULE_DEVICE_TABLE(of,bq27541_dt_ids);

static struct i2c_driver bq27541_battery_driver = {
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(bq27541_dt_ids),
	},
	.probe    = bq27541_battery_probe,
	.remove   = bq27541_battery_remove,
	.id_table = bq27541_id,
};

static int __init bq27541_battery_init(void){
	return i2c_add_driver(&bq27541_battery_driver);
}

static void __exit bq27541_battery_exit(void){
	i2c_del_driver(&bq27541_battery_driver);
}

module_init(bq27541_battery_init);
module_exit(bq27541_battery_exit);

MODULE_LICENSE("GPL");
