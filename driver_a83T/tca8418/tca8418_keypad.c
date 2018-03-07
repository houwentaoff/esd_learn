/*
 * Driver for TCA8418 I2C keyboard
 *
 * Copyright (C) 2011 Fuel7, Inc.  All rights reserved.
 *
 * Author: Kyle Manna <kyle.manna@fuel7.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 *
 * If you can't comply with GPLv2, alternative licensing terms may be
 * arranged. Please contact Fuel7, Inc. (http://fuel7.com/) for proprietary
 * alternative licensing inquiries.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/tca8418_keypad.h>
#include <linux/earlysuspend.h>

/* TCA8418 hardware limits */
#define TCA8418_MAX_ROWS	8
#define TCA8418_MAX_COLS	10

/* TCA8418 register offsets */
#define REG_CFG			0x01
#define REG_INT_STAT		0x02
#define REG_KEY_LCK_EC		0x03
#define REG_KEY_EVENT_A		0x04
#define REG_KEY_EVENT_B		0x05
#define REG_KEY_EVENT_C		0x06
#define REG_KEY_EVENT_D		0x07
#define REG_KEY_EVENT_E		0x08
#define REG_KEY_EVENT_F		0x09
#define REG_KEY_EVENT_G		0x0A
#define REG_KEY_EVENT_H		0x0B
#define REG_KEY_EVENT_I		0x0C
#define REG_KEY_EVENT_J		0x0D
#define REG_KP_LCK_TIMER	0x0E
#define REG_UNLOCK1		0x0F
#define REG_UNLOCK2		0x10
#define REG_GPIO_INT_STAT1	0x11
#define REG_GPIO_INT_STAT2	0x12
#define REG_GPIO_INT_STAT3	0x13
#define REG_GPIO_DAT_STAT1	0x14
#define REG_GPIO_DAT_STAT2	0x15
#define REG_GPIO_DAT_STAT3	0x16
#define REG_GPIO_DAT_OUT1	0x17
#define REG_GPIO_DAT_OUT2	0x18
#define REG_GPIO_DAT_OUT3	0x19
#define REG_GPIO_INT_EN1	0x1A
#define REG_GPIO_INT_EN2	0x1B
#define REG_GPIO_INT_EN3	0x1C
#define REG_KP_GPIO1		0x1D
#define REG_KP_GPIO2		0x1E
#define REG_KP_GPIO3		0x1F
#define REG_GPI_EM1		0x20
#define REG_GPI_EM2		0x21
#define REG_GPI_EM3		0x22
#define REG_GPIO_DIR1		0x23
#define REG_GPIO_DIR2		0x24
#define REG_GPIO_DIR3		0x25
#define REG_GPIO_INT_LVL1	0x26
#define REG_GPIO_INT_LVL2	0x27
#define REG_GPIO_INT_LVL3	0x28
#define REG_DEBOUNCE_DIS1	0x29
#define REG_DEBOUNCE_DIS2	0x2A
#define REG_DEBOUNCE_DIS3	0x2B
#define REG_GPIO_PULL1		0x2C
#define REG_GPIO_PULL2		0x2D
#define REG_GPIO_PULL3		0x2E

/* TCA8418 bit definitions */
#define CFG_AI			BIT(7)
#define CFG_GPI_E_CFG		BIT(6)
#define CFG_OVR_FLOW_M		BIT(5)
#define CFG_INT_CFG		BIT(4)
#define CFG_OVR_FLOW_IEN	BIT(3)
#define CFG_K_LCK_IEN		BIT(2)
#define CFG_GPI_IEN		BIT(1)
#define CFG_KE_IEN		BIT(0)

#define INT_STAT_CAD_INT	BIT(4)
#define INT_STAT_OVR_FLOW_INT	BIT(3)
#define INT_STAT_K_LCK_INT	BIT(2)
#define INT_STAT_GPI_INT	BIT(1)
#define INT_STAT_K_INT		BIT(0)

/* TCA8418 register masks */
#define KEY_LCK_EC_KEC		0x7
#define KEY_EVENT_CODE		0x7f
#define KEY_EVENT_VALUE		0x80

#define TCA8418_GPIO_INT GPIOH(8)

static __u32 twi_id = 0;

static const unsigned short normal_i2c[2] = {0x34,I2C_CLIENT_END};
static const struct i2c_device_id tca8418_id[] = {
	{ TCA8418_NAME, 8418, },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tca8418_id);
unsigned int config_rows = 3;
unsigned int config_cols = 3;

struct tca8418_keypad {
	unsigned int rows;
	unsigned int cols;
	unsigned int keypad_mask; /* Mask for keypad col/rol regs */
	unsigned int irq;
	unsigned int row_shift;

	struct i2c_client *client;
	struct input_dev *input;
	struct delayed_work work;
	struct early_suspend early_suspend;
	/* Flexible array member, must be at end of struct */
	unsigned short keymap[];
};

/*
 * Write a byte to the TCA8418
 */
static int tca8418_write_byte(struct tca8418_keypad *keypad_data,
			      int reg, u8 val)
{
	int error;

	error = i2c_smbus_write_byte_data(keypad_data->client, reg, val);
	if (error < 0) {
		dev_err(&keypad_data->client->dev,
			"%s failed, reg: %d, val: %d, error: %d\n",
			__func__, reg, val, error);
		return error;
	}

	return 0;
}

/*
 * Read a byte from the TCA8418
 */
static int tca8418_read_byte(struct tca8418_keypad *keypad_data,
			     int reg, u8 *val)
{
	int error;

	error = i2c_smbus_read_byte_data(keypad_data->client, reg);
	if (error < 0) {
		dev_err(&keypad_data->client->dev,
				"%s failed, reg: %d, error: %d\n",
				__func__, reg, error);
		return error;
	}

	*val = (u8)error;

	return 0;
}
static void tca8418_read_keypad(struct tca8418_keypad *keypad_data)
{
	int error, col, row;
	u8 reg, state, code;

	/* Initial read of the key event FIFO */
	error = tca8418_read_byte(keypad_data, REG_KEY_EVENT_A, &reg);

	/* Assume that key code 0 signifies empty FIFO */
	while (error >= 0 && reg > 0) {
		state = reg & KEY_EVENT_VALUE;
		code  = reg & KEY_EVENT_CODE;

		row = code / TCA8418_MAX_COLS;
		col = code % TCA8418_MAX_COLS;

		row = (col) ? row : row - 1;
		col = (col) ? col - 1 : TCA8418_MAX_COLS - 1;

		code = MATRIX_SCAN_CODE(row, col, keypad_data->row_shift);
		input_event(keypad_data->input, EV_MSC, MSC_SCAN, code);
		input_report_key(keypad_data->input,
				keypad_data->keymap[code], state);

		/* Read for next loop */
		error = tca8418_read_byte(keypad_data, REG_KEY_EVENT_A, &reg);
	}

	if (error < 0)
		dev_err(&keypad_data->client->dev,
			"unable to read REG_KEY_EVENT_A\n");

	input_sync(keypad_data->input);
}

/*
 * Threaded IRQ handler and this can (and will) sleep.
 */
static irqreturn_t tca8418_irq_handler(int irq, void *dev_id)
{
	struct tca8418_keypad *keypad_data = dev_id;
	u8 reg;
	int error;

	error = tca8418_read_byte(keypad_data, REG_INT_STAT, &reg);
	if (error) {
		dev_err(&keypad_data->client->dev,
			"unable to read REG_INT_STAT\n");
		goto exit;
	}

	if (reg & INT_STAT_OVR_FLOW_INT)
		dev_warn(&keypad_data->client->dev, "overflow occurred\n");

	if (reg & INT_STAT_K_INT){
		tca8418_read_keypad(keypad_data);
		tca8418_write_byte(keypad_data, REG_GPIO_DAT_OUT1, 0xf0);
		schedule_delayed_work(&keypad_data->work, 5000);
	
	}

exit:
	/* Clear all interrupts, even IRQs we didn't check (GPI, CAD, LCK) */
	reg = 0xff;
	error = tca8418_write_byte(keypad_data, REG_INT_STAT, reg);
	if (error)
		dev_err(&keypad_data->client->dev,
			"unable to clear REG_INT_STAT\n");

	return IRQ_HANDLED;
}

/*
 * Configure the TCA8418 for keypad operation
 */
static int __devinit tca8418_configure(struct tca8418_keypad *keypad_data)
{
	int reg, error;

	/* Write config register, if this fails assume device not present */
	error = tca8418_write_byte(keypad_data, REG_CFG,
				CFG_INT_CFG | CFG_OVR_FLOW_IEN | CFG_KE_IEN);
	if (error < 0)
		return -ENODEV;


	/* Assemble a mask for row and column registers */
	reg  =  ~(~0 << keypad_data->rows);
	reg += (~(~0 << keypad_data->cols)) << 8;
	keypad_data->keypad_mask = reg;

	/* Set registers to keypad mode */
	error |= tca8418_write_byte(keypad_data, REG_KP_GPIO1, reg);
	error |= tca8418_write_byte(keypad_data, REG_KP_GPIO2, reg >> 8);
	error |= tca8418_write_byte(keypad_data, REG_KP_GPIO3, reg >> 16);

	/* Enable column debouncing */
	error |= tca8418_write_byte(keypad_data, REG_DEBOUNCE_DIS1, reg);
	error |= tca8418_write_byte(keypad_data, REG_DEBOUNCE_DIS2, reg >> 8);
	error |= tca8418_write_byte(keypad_data, REG_DEBOUNCE_DIS3, reg >> 16);

	tca8418_write_byte(keypad_data, REG_GPIO_DIR1 , 0xf0);
	tca8418_write_byte(keypad_data, REG_GPIO_DIR2 , 0xf0);
	tca8418_write_byte(keypad_data, REG_GPIO_PULL1 , 0x0f);
	tca8418_write_byte(keypad_data, REG_GPIO_PULL2 , 0x0f);

	return error;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tca8418_early_suspend(struct early_suspend *h)
{
	struct tca8418_keypad *keypad_data;
	keypad_data = container_of(h, struct tca8418_keypad, early_suspend);

	cancel_delayed_work_sync(&keypad_data->work);
 	
}

static void tca8418_late_resume(struct early_suspend *h)
{
	struct tca8418_keypad *keypad_data;

	keypad_data = container_of(h, struct tca8418_keypad, early_suspend);
	tca8418_configure(keypad_data);
	
  
}
#else
#define  keyboard_event_suspend NULL
#define keyboard_event_resume NULL
#endif
static void tca8418_work(struct work_struct *work){
	struct tca8418_keypad *chip;
	struct tca8418_keypad *keypad_data;

	chip = container_of(work, struct tca8418_keypad, work.work);
	keypad_data = i2c_get_clientdata(chip->client);

	tca8418_write_byte(keypad_data, REG_GPIO_DAT_OUT1, 0x0f);

}
static int keypad_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
        
        if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
                return -ENODEV;
	if(twi_id == adapter->nr){
       strlcpy(info->type, TCA8418_NAME, I2C_NAME_SIZE);
				
		return 0;

	}else{
		printk("%s detect fail\n", __func__);
		return -ENODEV;
	}
}
static  uint32_t   tca8418_keymap[] __initdata={

//
       KEY(0,0,KEY_RIGHT),KEY(0,1,KEY_BACK),KEY(0,2,KEY_UP),
   	KEY(1,0,KEY_ENTER),KEY(1,1,KEY_CAMERA),KEY(1,2,KEY_DOWN),
   	KEY(2,0,KEY_BACKSPACE),KEY(2,1,KEY_NUMLOCK),KEY(2,2,KEY_LEFT),


};

 static struct matrix_keymap_data keymap_data={

       .keymap=tca8418_keymap,
	 .keymap_size= ARRAY_SIZE( tca8418_keymap),	
 };



static int __devinit tca8418_keypad_probe(struct i2c_client *client,
					  const struct i2c_device_id *id)
{
	
	struct tca8418_keypad *keypad_data;
	struct input_dev *input;
	int error, row_shift, max_keys;

	

	/* Check i2c driver capabilities */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE)) {
		dev_err(&client->dev, "%s adapter not supported\n",
			dev_driver_string(&client->adapter->dev));
		return -ENODEV;
	}

	row_shift = get_count_order(config_cols);
	max_keys = config_rows << row_shift;

	/* Allocate memory for keypad_data, keymap and input device */
	keypad_data = kzalloc(sizeof(*keypad_data) +
			max_keys * sizeof(keypad_data->keymap[0]), GFP_KERNEL);
	if (!keypad_data)
		return -ENOMEM;

	keypad_data->rows = config_rows;
	keypad_data->cols = config_cols;
	keypad_data->client = client;
	keypad_data->row_shift = row_shift;

	/* Initialize the chip or fail if chip isn't present */
	error = tca8418_configure(keypad_data);
	if (error < 0)
		goto fail1;

	/* Configure input device */
	input = input_allocate_device();
	if (!input) {
		error = -ENOMEM;
		goto fail1;
	}
	keypad_data->input = input;

	input->name = client->name;
	input->dev.parent = &client->dev;

	input->id.bustype = BUS_I2C;
	input->id.vendor  = 0x0001;
	input->id.product = 0x001;
	input->id.version = 0x0001;

	input->keycode     = keypad_data->keymap;
	input->keycodesize = sizeof(keypad_data->keymap[0]);
	input->keycodemax  = max_keys;

#ifdef CONFIG_HAS_EARLYSUSPEND
	keypad_data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    keypad_data->early_suspend.suspend = tca8418_early_suspend;
	keypad_data->early_suspend.resume = tca8418_late_resume;
	register_early_suspend(&keypad_data->early_suspend);
#endif
	__set_bit(EV_KEY, input->evbit);

	__set_bit(EV_REP, input->evbit);

	input_set_capability(input, EV_MSC, MSC_SCAN);

	input_set_drvdata(input, keypad_data);

	matrix_keypad_build_keymap(&keymap_data, row_shift,
			input->keycode, input->keybit);
	
	gpio_request_one(TCA8418_GPIO_INT, GPIOF_IN, TCA8418_NAME);

	client->irq = gpio_to_irq(TCA8418_GPIO_INT);

	error = request_threaded_irq(client->irq, NULL, tca8418_irq_handler,
				     IRQF_TRIGGER_FALLING,
				     client->name, keypad_data);
	if (error) {
		dev_dbg(&client->dev,
			"Unable to claim irq %d; error %d\n",
			client->irq, error);
		goto fail2;
	}

	error = input_register_device(input);
	if (error) {
		dev_dbg(&client->dev,
			"Unable to register input device, error: %d\n", error);
		goto fail3;
	}
	
	INIT_DELAYED_WORK(&keypad_data->work, tca8418_work);

	i2c_set_clientdata(client, keypad_data);
	return 0;

fail3:
	free_irq(client->irq, keypad_data);
fail2:
	input_free_device(input);
fail1:
	kfree(keypad_data);
	return error;
}

static int __devexit tca8418_keypad_remove(struct i2c_client *client)
{
	struct tca8418_keypad *keypad_data = i2c_get_clientdata(client);

	free_irq(keypad_data->client->irq, keypad_data);

	input_unregister_device(keypad_data->input);

	kfree(keypad_data);

	return 0;
}


static struct i2c_driver tca8418_keypad_driver = {
	.class          = I2C_CLASS_HWMON,
	.driver = {
		.name	= TCA8418_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= tca8418_keypad_probe,
	.remove		= __devexit_p(tca8418_keypad_remove),
	.id_table	= tca8418_id,
	.address_list	= normal_i2c,
};

static int __init tca8418_keypad_init(void)
{
	 tca8418_keypad_driver.detect  = keypad_detect;
	return i2c_add_driver(&tca8418_keypad_driver);
}
late_initcall(tca8418_keypad_init);

static void __exit tca8418_keypad_exit(void)
{
	i2c_del_driver(&tca8418_keypad_driver);
}
module_exit(tca8418_keypad_exit);

MODULE_AUTHOR("Kyle Manna <kyle.manna@fuel7.com>");
MODULE_DESCRIPTION("Keypad driver for TCA8418");
MODULE_LICENSE("GPL");
