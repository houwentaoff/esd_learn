/*************************************************************************
	> File Name: ch452_keyboard.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2018年3月23日 星期一 10时31分57秒
 ************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/input/matrix_keypad.h>
#include <linux/platform_device.h>

static struct i2c_client *g_client_ch452;
struct input_dev *g_input_dev;
static int ch452_scl ;
static int ch452_sda ;
static int ch452_int ;

#define DRIVER_NAME "ch452_keyboard"
#define CH452_I2C_MASK 0x3E
#define CH452_VER 0x0040
#define IIC_DELAY 10//延时
#define CH452_I2C_ADDR 0x60
#define CH452_GET_KEY 0x0700 //获取按键,返回按键代码
#define CH452_SYSON 0x0402 //开启键盘
#define CH452_ROW_SHIFT 3
#define MAX_ROW 8
#define MAX_COL 8

#define CH452_SCL_SET		gpio_set_value(ch452_scl, 1)
#define CH452_SCL_CLR		gpio_set_value(ch452_scl, 0)
#define CH452_SDA_SET		gpio_set_value(ch452_sda, 1)
#define CH452_SDA_CLR		gpio_set_value(ch452_sda, 0)
#define CH452_SDA_IN		gpio_get_value(ch452_sda)
#define CH452_SDA_D_OUT		gpio_direction_output(ch452_sda,1)
#define CH452_SDA_D_IN		gpio_direction_input(ch452_sda)


/**
 * 硬件直接连接的是gpio摸拟i2c,即scl_pin sda_pin int_pin
 * 如果直接连cpu的i2c总线怒标准的２线接口,sda应和ch452的int相连
 * sda做中断输出
 */

//keycodes是linux键盘映射之后的值比如back 映射后为158
//include/uapi/linux/input.h
unsigned short keycodes[64];

static const unsigned int ch452_keymap[] = {
	KEY(3, 2, KEY_BACK),
	KEY(2,0, KEY_DOWN),
	KEY(2,1, KEY_UP),
};

static const struct matrix_keymap_data keymap_data = {
	.keymap = ch452_keymap,
	.keymap_size = ARRAY_SIZE(ch452_keymap),
};

void ch452_i2c_start(void){
	CH452_SDA_D_OUT;//设置SDA为输出方向
	CH452_SDA_SET;//发送起始条件的数据信号
	CH452_SCL_SET;
	udelay(IIC_DELAY);
	CH452_SDA_CLR; //发送起始信号
	udelay(IIC_DELAY);
	CH452_SCL_CLR;//钳住i2c总线,准备发送或接收数据
	udelay(IIC_DELAY);
}

void ch452_i2c_stop(void){
	CH452_SDA_D_OUT;//设置SDA为输出方向
	CH452_SDA_CLR;
	udelay(IIC_DELAY);
	CH452_SCL_SET;
	udelay(IIC_DELAY);
	CH452_SDA_SET;//发送i2c总线结束信号
	udelay(IIC_DELAY);
	udelay(IIC_DELAY);
	udelay(IIC_DELAY);
	udelay(IIC_DELAY);
}
//写一个字节数据
void ch452_i2c_write_byte(unsigned char dat){
	unsigned char i;
	CH452_SDA_D_OUT;//设置SDA为输出方向
	for(i=0;i!=8;i++){//输出8位数据
		if(dat & 0x80){
			CH452_SDA_SET;
		}else{
            CH452_SDA_CLR;
		}
		udelay(IIC_DELAY);
		CH452_SCL_SET;
		dat<<=1;
		udelay(IIC_DELAY);
		CH452_SCL_CLR;
		udelay(IIC_DELAY);
	}
	CH452_SDA_SET;
	udelay(IIC_DELAY);
	CH452_SCL_SET;//接收应答
	udelay(IIC_DELAY);
	CH452_SCL_CLR;
	udelay(IIC_DELAY);

}

//读一个字节数据
unsigned char ch452_i2c_read_byte(void){
	unsigned char dat,i;
	CH452_SDA_SET;
	udelay(IIC_DELAY);
	CH452_SDA_D_IN;
	dat = 0;
	for(i=0;i!=8;i++){//输入８位数
		CH452_SCL_SET;
		udelay(IIC_DELAY);
		dat<<=1;
		if(CH452_SDA_IN)
		  dat++;
		CH452_SCL_CLR;
		udelay(IIC_DELAY);
	}
	CH452_SDA_D_OUT;
	CH452_SDA_SET;
	udelay(IIC_DELAY);
	CH452_SCL_SET;//发出无效应答
	udelay(IIC_DELAY);
	CH452_SCL_CLR;
	udelay(IIC_DELAY);

	return dat;
}
//写命令
void ch452_write(unsigned short cmd){
	ch452_i2c_start();//启动总线
	ch452_i2c_write_byte(((cmd>>7)&0xff)&CH452_I2C_MASK | CH452_I2C_ADDR);
	ch452_i2c_write_byte((unsigned char)cmd);
	ch452_i2c_stop();
}
//读取按键
unsigned char ch452_read(void){
	unsigned char keycode;
	ch452_i2c_start();
	ch452_i2c_write_byte((unsigned char)(CH452_GET_KEY>>7)&CH452_I2C_MASK |
		0x01 | CH452_I2C_ADDR);
	keycode = ch452_i2c_read_byte();//读取数据
	ch452_i2c_stop;

	return keycode;
}


static int ch452_init(void){
	unsigned char version = 0xff;
	int i=0;
	struct device_node *dev_node = NULL;
	//获取中断引脚
	dev_node = of_find_compatible_node(NULL,NULL,"fsl,ch452");
	if(!of_device_is_compatible(dev_node,"fsl,ch452")){
	
		return -1;
	}
	ch452_scl = of_get_named_gpio(dev_node, "i2c_scl", 0);
	ch452_sda = of_get_named_gpio(dev_node, "i2c_sda", 0);
	ch452_int = of_get_named_gpio(dev_node,"key_int",0);

	devm_gpio_request_one(&g_client_ch452->dev,ch452_scl,GPIOF_OUT_INIT_HIGH, "scl pin");
	devm_gpio_request_one(&g_client_ch452->dev,ch452_sda,GPIOF_OUT_INIT_HIGH,"sda pin");
	devm_gpio_request_one(&g_client_ch452->dev,ch452_int, GPIOF_IN, "key int");

	//只开启键盘
	ch452_write(CH452_SYSON);
//获取版本
	while(i<10){
	ch452_i2c_start();
	ch452_i2c_write_byte((((CH452_VER>>7)&0xff)&CH452_I2C_MASK)|0x01|CH452_I2C_ADDR);
	version = ch452_i2c_read_byte();
	printk("version:0x%x\n",version);
	ch452_i2c_stop();
	if(version!=0xff)
	  break;
	i++;
}

	return 0;
}
//关于ch452_row_shift是指移到下一行的个数
//这里最大行为8 ,移到下一行要移动2的3次方个
static irqreturn_t ch452_irq_handler(int irq, void *dev_id){
	int val,row,col,code,release;
	static int keystate = 0;

	val = ch452_read();//读走键值,才响应下次中断
	//获取行,列,和按键码
	
	row = val & 0x07;
	col = (val >> CH452_ROW_SHIFT) & 0x07;
	code = MATRIX_SCAN_CODE(row, col,CH452_ROW_SHIFT);
	
	release = keystate ? 1:0;
	keystate = ~keystate;
	printk("row:%d col:%d code:0x%x keycodes[code]:%d\n", row, col, code, keycodes[code]);
	//上报数据
	input_event(g_input_dev, EV_MSC, MSC_SCAN ,code);
	input_report_key(g_input_dev, keycodes[code], !release);
	input_sync(g_input_dev);

	return IRQ_HANDLED;
}

static int ch452_keyboard_probe(struct i2c_client *client,
			const struct i2c_device_id *id){
	int ret;

	struct input_dev *input;

	input = devm_input_allocate_device(&client->dev);

	input->name = client->name;
	input->id.bustype = BUS_I2C;

	g_input_dev = input;

	matrix_keypad_build_keymap(&keymap_data, NULL,MAX_ROW,MAX_COL,keycodes,input);

	input_set_capability(input, EV_MSC, MSC_SCAN);

	ret = input_register_device(input);
	if(ret){
		input_free_device(input);
	}

	g_client_ch452 = client;
	
	ret = ch452_init();
	if(ret<0){
		return -1;
	}
	ret = devm_request_threaded_irq(
				&client->dev,
				gpio_to_irq(ch452_int),
				NULL,
				ch452_irq_handler,
				IRQF_TRIGGER_FALLING | IRQF_ONESHOT | IRQF_TRIGGER_RISING,
				client->name,
				NULL);
	if(ret)
	  return -1;

	return 0;

}

static int ch452_keyboard_remove(struct i2c_client *client){
	
	printk("bshui ch452_keyboard_remove---\n");
	return 0;
}


static const struct i2c_device_id ch452_id[] = {
	{DRIVER_NAME, 0},
	{ }
};

static const struct of_device_id ch452_dt_ids[] = {
	{ .compatible = "fsl,ch452",},
	{ }
};
MODULE_DEVICE_TABLE(of,ch452_dt_ids);

static struct i2c_driver ch452_keyboard_driver = {
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ch452_dt_ids),
	},
	.probe    = ch452_keyboard_probe,
	.remove   = ch452_keyboard_remove,
	.id_table = ch452_id,
};

static int __init ch452_keyboard_init(void){
	return i2c_add_driver(&ch452_keyboard_driver);
}

static void __exit ch452_keyboard_exit(void){
	i2c_del_driver(&ch452_keyboard_driver);
}

module_init(ch452_keyboard_init);
module_exit(ch452_keyboard_exit);

MODULE_LICENSE("GPL");
