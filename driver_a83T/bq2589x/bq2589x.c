/*************************************************************************
	> File Name: bq2589x.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年09月13日 星期二 13时32分44秒
 ************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <mach/sys_config.h>

#define BQ2589X_REG00 0x00
#define EN_ILIM_MASK BIT(6)
#define EN_ILIM_SHIFT 6
#define IINLIM_MASK  (BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))
#define IINLIM_SHIFT 0

#define BQ2589X_REG01 0x01
#define VINDPM_MASK (BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))
#define VINDPM_SHIFT 0

#define BQ2589X_REG02 0x02
#define CONV_START_MASK BIT(7)
#define CONV_START_SHIFT 7
#define FORCE_DPDM_MASK BIT(1)
#define FORCE_DPDM_SHIFT 1
#define AUTO_DPDM_MASK BIT(0)
#define AUTO_DPDM_SHIFT 0

#define BQ2589X_REG03 0x03
#define OTG_CONFIG_MASK BIT(5)
#define OTG_CONFIG_SHIFT 5
#define CHG_CONFIG_MASK BIT(4)
#define CHG_CONFIG_SHIFT 4

#define BQ2589X_REG04 0x04
#define ICHG_MASK (BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))
#define ICHG_SHIFT 0

#define BQ2589X_REG05 0x05

#define BQ2589X_REG06 0x06
#define VREG_MASK (BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2))
#define VREG_SHIFT 2

#define BQ2589X_REG07 0x07
#define WATCHDOG_MASK (BIT(5)|BIT(4))
#define WATCHDOG_SHIFT 4

#define BQ2589X_REG08 0x08
#define BAT_COMP_MASK (BIT(7)|BIT(6)|BIT(5))
#define BAT_COMP_SHIFT 5
#define VCLAMP_MASK (BIT(4)|BIT(3)|BIT(2))
#define VCLAMP_SHIFT 2

#define BQ2589X_REG09 0x09
#define BATFET_DIS_MASK BIT(5)
#define BATFET_DIS_SHIFT 5
#define BATFET_DLY_MASK BIT(3)
#define BATFET_DLY_SHIFT 3

#define BQ2589X_REG0B 0x0B
#define CHRG_STAT_MASK (BIT(4)|BIT(3))
#define CHRG_STAT_SHIFT 3
#define PG_STAT_MASK BIT(2)
#define PG_STAT_SHIFT 2

#define BQ2589X_REG11 0x11
#define VBUSV_MASK_6 BIT(6)
#define VBUSV_SHIFT_6 6
#define VBUSV_MASK_5 BIT(5)
#define VBUSV_SHIFT_5 5
#define VBUSV_MASK_4 BIT(4)
#define VBUSV_SHIFT_4 4
#define VBUSV_MASK_3 BIT(3)
#define VBUSV_SHIFT_3 3
#define VBUSV_MASK_2 BIT(2)
#define VBUSV_SHIFT_2 2
#define VBUSV_MASK_1 BIT(1)
#define VBUSV_SHIFT_1 1
#define VBUSV_MASK_0 BIT(0)
#define VBUSV_SHIFT_0 0

#define BQ2589X_REG14 0x14
#define REG_RST_MASK BIT(7)
#define REG_RST_SHIFT 7

//#define USB_ID_GPIO    GPIOH(11)
//#define BQ25890_GPIO_INT  GPIOG(12)

#define BQ25890_NAME "bq25890_charger"
#define BQ25892_NAME "bq25892_charger"

static int bq25890_gpio_int = 0;
static int usb_id_gpio = 0;
struct bq2589x_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct power_supply charger;
	struct delayed_work work;
	int chip_type;
	unsigned int irq;
	unsigned char watchdog;
	
};

struct bq2589x_dev_info *G_bdi_890;
struct bq2589x_dev_info *G_bdi_892;



static const unsigned short bq25890_i2c[] = {0x6a,I2C_CLIENT_END};
static const unsigned short bq25892_i2c[] = {0x6b,I2C_CLIENT_END};
enum chips{bq25890, bq25892};



static u8 bq2589x_reg_read(struct i2c_client *client, u8 reg){

	return i2c_smbus_read_byte_data(client, reg);
}

static int bq2589x_reg_write(struct i2c_client *client, u8 reg, unsigned int value){
	
	return i2c_smbus_write_byte_data(client, reg, value);
}

static int bq2589x_read_mask(struct i2c_client *client, u8 reg,
			u8 mask, u8 shift){
	u8 value;
	value = bq2589x_reg_read(client, reg);

	value &=mask;
	value >>=shift;
	return value;	
}

static int bq2589x_write_mask(struct i2c_client *client, u8 reg, u8 mask,
			u8 shift, u8 data){
	u8 value;
	value = bq2589x_reg_read(client, reg);

	value &=~mask;
	value |= ((data<<shift)&mask);

	return bq2589x_reg_write(client, reg, value);
}

static int bq2589x_register_reset(struct bq2589x_dev_info *bdi){
	int ret,limit=100;
	u8 v;
	/*Reset the registers*/
	ret = bq2589x_write_mask(bdi->client, BQ2589X_REG14, REG_RST_MASK,
				REG_RST_SHIFT, 0x01);
	if(ret < 0)
	  return ret;
	/*Reset bit will be cleared by hardware so poll until it is*/
	do{
		v = bq2589x_read_mask(bdi->client, BQ2589X_REG14,
					REG_RST_MASK,REG_RST_SHIFT);
		if(!v)
			break;
		udelay(10);
	}while(--limit);

	return 0;
}

static void bq2589x_hw_init(struct bq2589x_dev_info *bdi){
	/*reset the register befor init*/
	bq2589x_register_reset(bdi);
	/*Disable ILIM Pin*/
	bq2589x_write_mask(bdi->client, BQ2589X_REG00, EN_ILIM_MASK,
				EN_ILIM_SHIFT, 0x00);
	/*Fast Charge Current Limit 1472mA*/
	bq2589x_write_mask(bdi->client, BQ2589X_REG04, ICHG_MASK,
				ICHG_SHIFT, 0x17 );
	/*Charge Voltage Limit 4.35V*/
	bq2589x_write_mask(bdi->client, BQ2589X_REG06, VREG_MASK,
				VREG_SHIFT, 0x20);
	/*write watchdog 00 to diable watchdog timer*/
	bq2589x_write_mask(bdi->client, BQ2589X_REG07, WATCHDOG_MASK, 
				WATCHDOG_SHIFT, 0x00);
	/*IR Compensation Resistor 40mO*/
	bq2589x_write_mask(bdi->client, BQ2589X_REG08, BAT_COMP_MASK,
				BAT_COMP_SHIFT, 0x02);
	/*IR Compensation Voltage 128mV*/
	bq2589x_write_mask(bdi->client, BQ2589X_REG08, VCLAMP_MASK,
				VCLAMP_SHIFT, 0x04);

}
static irqreturn_t bq25890_irq_handler(int irq, void *data){
	
	struct bq2589x_dev_info *bdi = data;

	power_supply_changed(&bdi->charger);

	return IRQ_HANDLED;
}

static void bq25890_work(struct work_struct *work){
	struct bq2589x_dev_info *chip;
	struct bq2589x_dev_info *bdi;
	int level = 2;
	chip = container_of(work, struct bq2589x_dev_info, work.work);
	bdi = i2c_get_clientdata(chip->client);
	level = gpio_get_value(usb_id_gpio);
	if(level == 0){
		//boost 
		bq2589x_write_mask(bdi->client, BQ2589X_REG03, OTG_CONFIG_MASK,
			OTG_CONFIG_SHIFT, 0x1);
	}else if(level ==1){
		//reset default value
		bq2589x_write_mask(bdi->client, BQ2589X_REG03, OTG_CONFIG_MASK,
			OTG_CONFIG_SHIFT, 0x0);
	}

	schedule_delayed_work(&bdi->work, 100);
}

static void bq25892_work(struct work_struct *work){
	struct bq2589x_dev_info *chip;
	struct bq2589x_dev_info *bdi;
	int level = 2;
	chip = container_of(work, struct bq2589x_dev_info, work.work);
	bdi = i2c_get_clientdata(chip->client);
	level = gpio_get_value(usb_id_gpio);
	if(level == 0){
		//boost 
		bq2589x_write_mask(bdi->client, BQ2589X_REG03, CHG_CONFIG_MASK,
			CHG_CONFIG_SHIFT, 0x0);
	}else if(level ==1){
		//reset default value
		bq2589x_write_mask(bdi->client, BQ2589X_REG03, CHG_CONFIG_MASK,
			CHG_CONFIG_SHIFT, 0x1);
	}
	schedule_delayed_work(&bdi->work, 100);
}


static int bq25890_charger_get_online(struct bq2589x_dev_info *bdi, 
			union power_supply_propval *val){

	u8 v;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG0B,
				PG_STAT_MASK, PG_STAT_SHIFT);
	printk("bq25890 get online = %d\n", v);
	val->intval = v;
	
	return 0;
}

static int bq25890_charger_get_status(struct bq2589x_dev_info *bdi,
			union power_supply_propval *val){

	u8 v;
	int status;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG0B, CHRG_STAT_MASK,
				CHRG_STAT_SHIFT);
	switch(v){
		case 0x00:
			status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
		case 0x01://Pre-charge
		case 0x02://Fast Charging
			status = POWER_SUPPLY_STATUS_CHARGING;
		break;
		case 0x03://Charge Termination Done
			status = POWER_SUPPLY_STATUS_FULL;
		break;
		default:
			return -EIO;
	
	}

	val->intval = status;
	printk("bq2589x status=%d\n", status);
	return 0;
}

static int bq25890_charger_get_voltage(struct bq2589x_dev_info *bdi,
			union power_supply_propval *val){
	int v_base = 2600;
	u8 v = 0;
	int voltage = 0;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG11,
				VBUSV_MASK_6,VBUSV_SHIFT_6);
	if(v)
	  voltage = 6400;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG11,
				VBUSV_MASK_5,VBUSV_SHIFT_5);
	if(v)
	  voltage += 3200;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG11,
				VBUSV_MASK_4,VBUSV_SHIFT_4);
	if(v)
	  voltage += 1600;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG11,
				VBUSV_MASK_3,VBUSV_SHIFT_3);
	if(v)
	  voltage += 800;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG11,
				VBUSV_MASK_2,VBUSV_SHIFT_2);
	if(v)
	  voltage += 400;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG11,
				VBUSV_MASK_1,VBUSV_SHIFT_1);
	if(v)
	  voltage += 200;

	v = bq2589x_read_mask(bdi->client, BQ2589X_REG11,
				VBUSV_MASK_0,VBUSV_SHIFT_0);
	if(v)
	  voltage += 100;

	voltage += v_base;
	
	
	printk("bq2589x voltage=%dmV\n", voltage);
	val->intval = voltage;
	return 0;
}
static int bq25890_charger_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val){

	struct bq2589x_dev_info *bdi = container_of(psy, struct bq2589x_dev_info,
				charger);

	switch(psp){
		case POWER_SUPPLY_PROP_ONLINE:
			 bq25890_charger_get_online(bdi, val);
		break;
		case POWER_SUPPLY_PROP_STATUS:
			bq25890_charger_get_status(bdi, val);
		break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			bq25890_charger_get_voltage(bdi, val);
		break;
		default:
			return -EINVAL;
	
	}

	return 0;
}

static enum power_supply_property bq25890_charger_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static int bq2589x_probe(struct i2c_client *client, const struct i2c_device_id *id){
	int ret = 0;
	struct device *dev = &client->dev;
	struct bq2589x_dev_info *bdi_890;
	struct bq2589x_dev_info *bdi_892;
	script_item_value_type_e type = 0;
	script_item_u item_temp;
	

	type = script_get_item("bq2589x_para", "bq2589x_used", &item_temp);

	if(SCIRPT_ITEM_VALUE_TYPE_INT != type){
		printk("bq2589x usb charger type error \n");
		return -1;
	}
	
	if(item_temp.val == 0){
		printk("bq2589x usb charger type not used\n");
		return -1;
	}

	type = script_get_item("bq2589x_para", "bq2589x_irq", &item_temp);
	if(SCIRPT_ITEM_VALUE_TYPE_PIO != type){
		printk("bq2589x irq gpio failed\n");
		return -1;
	}else{
		bq25890_gpio_int = item_temp.gpio.gpio;
	}

	type = script_get_item("usbc0", "usb_id_gpio", &item_temp);
	if(SCIRPT_ITEM_VALUE_TYPE_PIO != type){
		printk("bq2589x usb id gpio failed\n");
		return -1;
	}else{
		usb_id_gpio = item_temp.gpio.gpio;
	}
	
	if(id->driver_data == 0){

		bdi_890 = devm_kzalloc(dev, sizeof(*bdi_890), GFP_KERNEL);
		if(!bdi_890){
			printk("bq25890 cannot allocate bdi memory\n");
			return -ENODEV;
		}

		bdi_890->client = client;
		bdi_890->dev = dev;
		bdi_890->chip_type = id->driver_data;
		i2c_set_clientdata(client, bdi_890);

		bq2589x_hw_init(bdi_890);

		gpio_request_one(bq25890_gpio_int, GPIOF_IN, BQ25890_NAME);
		gpio_request_one(usb_id_gpio, GPIOF_IN, "usb-id");
		ret = devm_request_threaded_irq(bdi_890->dev, gpio_to_irq(bq25890_gpio_int),
				NULL,bq25890_irq_handler, IRQF_TRIGGER_FALLING,"bq25890-charger",bdi_890);

		if(ret < 0){
			printk("bq25890 cant set up irq handler\n");
			return ret;
		}

		/*charger init*/
		bdi_890->charger.name = BQ25890_NAME;
		bdi_890->charger.type = POWER_SUPPLY_TYPE_USB;
		bdi_890->charger.properties = bq25890_charger_properties;
		bdi_890->charger.num_properties = ARRAY_SIZE(bq25890_charger_properties);
		bdi_890->charger.get_property = bq25890_charger_get_property;
		
		ret = power_supply_register(bdi_890->dev, &bdi_890->charger);
		if(ret){
			printk("Can't register charger bq25890\n");
			gpio_free(bq25890_gpio_int);
			return ret;
		}
		INIT_DELAYED_WORK(&bdi_890->work, bq25890_work);
		schedule_delayed_work(&bdi_890->work, 1000);

	}else if(id->driver_data == 1){
	
		bdi_892 = devm_kzalloc(dev, sizeof(*bdi_892), GFP_KERNEL);
		if(!bdi_892){
			printk("bq25892 cannot allocate bdi memory\n");
			return -ENODEV;
		}

		bdi_892->client = client;
		bdi_892->dev = dev;
		bdi_892->chip_type = id->driver_data;
		i2c_set_clientdata(client, bdi_892);
		bq2589x_hw_init(bdi_892);
		INIT_DELAYED_WORK(&bdi_892->work, bq25892_work);
		schedule_delayed_work(&bdi_892->work, 1000);
	
	}

	return ret;
}

static int bq25890_remove(struct i2c_client *client){
	
	struct bq2589x_dev_info *bdi = i2c_get_clientdata(client);
		bq2589x_register_reset(bdi);
		power_supply_unregister(&bdi->charger);
		cancel_delayed_work_sync(&bdi->work);
		free_irq(gpio_to_irq(bq25890_gpio_int),bdi);
		kfree(bdi);
		gpio_free(bq25890_gpio_int);

	return 0;
}

static int bq25892_remove(struct i2c_client *client){
	
	struct bq2589x_dev_info *bdi = i2c_get_clientdata(client);
	bq2589x_register_reset(bdi);
	gpio_free(usb_id_gpio);
	kfree(bdi);
	return 0;
}
static struct i2c_device_id bq25890_id[] = {
	{BQ25890_NAME, bq25890},
	{}
};

static struct i2c_device_id bq25892_id[] = {
	{BQ25892_NAME, bq25892},
	{}
};

static int bq25890_suspend(struct i2c_client *client, pm_message_t state){
	struct bq2589x_dev_info *bdi = i2c_get_clientdata(client);
	cancel_delayed_work_sync(&bdi->work);
	return 0;
}
static int bq25892_suspend(struct i2c_client *client, pm_message_t state){
	struct bq2589x_dev_info *bdi = i2c_get_clientdata(client);
	cancel_delayed_work_sync(&bdi->work);
	return 0;
}

static int bq25890_resume(struct i2c_client *client){
	struct bq2589x_dev_info *bdi = i2c_get_clientdata(client);
	schedule_delayed_work(&bdi->work, 100);
	return 0;
}

static int bq25892_resume(struct i2c_client *client){
	struct bq2589x_dev_info *bdi = i2c_get_clientdata(client);
	schedule_delayed_work(&bdi->work, 100);
	return 0;
}

static struct i2c_driver bq25890_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name=BQ25890_NAME,
		.owner=THIS_MODULE,
	},
	.probe = bq2589x_probe,
	.remove = bq25890_remove,
	.suspend = bq25890_suspend,
	.resume  = bq25890_resume,
	.id_table = bq25890_id,
	.address_list = bq25890_i2c,

};

static struct i2c_driver bq25892_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name=BQ25892_NAME,
		.owner=THIS_MODULE,
	},
	.probe = bq2589x_probe,
	.remove = bq25892_remove,
	.suspend = bq25892_suspend,
	.resume  = bq25892_resume,
	.id_table = bq25892_id,
	.address_list = bq25892_i2c,

};

static int bq25890_detect(struct i2c_client *client, struct i2c_board_info *info){
	struct i2c_adapter *adapter = client->adapter;
	int twi_id = 1;

	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
	  return -ENODEV;

	if(twi_id == adapter->nr){
		
		//printk("ba25890 %s addr:0x%x\n", __func__, client->addr);
		strlcpy(info->type, BQ25890_NAME, I2C_NAME_SIZE);
		return 0;
	}else{
		//printk("bq25890 %s detect fail\n", __func__);
		return -ENODEV;
	}
}

static int bq25892_detect(struct i2c_client *client, struct i2c_board_info *info){
	struct i2c_adapter *adapter = client->adapter;
	int twi_id = 1;

	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
	  return -ENODEV;

	if(twi_id == adapter->nr){
		
		//printk("ba25892 %s addr:0x%x\n", __func__, client->addr);
		strlcpy(info->type, BQ25892_NAME, I2C_NAME_SIZE);
		return 0;
	}else{
		//printk("bq25892 %s detect fail\n", __func__);
		return -ENODEV;
	}
	
}
static int __init bq2589x_init(void){
	int ret = -1;

	bq25890_driver.detect = bq25890_detect;
	bq25892_driver.detect = bq25892_detect;

	ret = i2c_add_driver(&bq25890_driver);
	if(ret){
		//printk("bq25890 unable to register driver\n");
	}else{
		//printk("bq25890 success register driver\n");
	}

	ret = i2c_add_driver(&bq25892_driver);
	if(ret){
		//printk("bq25892 unable to register driver\n");
	}else{
		//printk("bq25892 success register driver\n");
	}

	return ret;
}

static void __exit bq2589x_exit(void){
	
	i2c_del_driver(&bq25890_driver);
	i2c_del_driver(&bq25892_driver);
}
module_init(bq2589x_init);
module_exit(bq2589x_exit);
MODULE_LICENSE("GPL");




