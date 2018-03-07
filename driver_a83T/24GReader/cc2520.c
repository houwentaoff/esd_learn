/*************************************************************************
	> File Name: cc2520.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年11月07日 星期一 15时26分25秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <mach/irqs.h>
#include <linux/slab.h>
//#include "2520.h"

#define SPI_COMMAND_BUFFER 3

#define GPIO1_FIFO  GPIOE(19)
#define GPIO2_FIFOP GPIOB(3)
#define GPIO4_SFD   GPIOB(2)
#define GPIO_VREG_EN    SUNXI_EXP_PIN_BASE+0
#define GPIO_RESET_EN   SUNXI_EXP_PIN_BASE+2
#define GPIO_TX			SUNXI_EXP_PIN_BASE+9
#define GPIO_2591_PAEN  SUNXI_EXP_PIN_BASE+13
#define GPIO_2591_RXEN  SUNXI_EXP_PIN_BASE+14
#define GPIO_2591_HGMEN SUNXI_EXP_PIN_BASE+15

#define HIGH 1
#define LOW  0
#define STATE_IDLE 0
#define RSSI_VALID 0
#define RSSI_OFFSET 78

#if 1
#define CC2520_FREG_MASK 0x3F
#define CC2520_RAM_SIZE 640
#define CC2520_FIFO_SIZE 128

#define CC2520_CMD_SNOP 0x00
#define CC2520_CMD_MEMORY_READ  0x10
#define CC2520_CMD_MEMORY_WRITE 0x20
#define CC2520_FSMSTAT1 0x33
#define CC2520_TXPOWER 0x30
#define CC2520_CCACTRL0 0x36
#define CC2520_MDMCTRL0 0x46
#define CC2520_MDMCTRL1 0x47
#define CC2520_RXCTRL 0x4a
#define CC2520_FSCTRL 0x4c
#define CC2520_FSCAL1 0x4f
#define CC2520_AGCCTRL1 0x53
#define CC2520_ADCTEST0 0x56
#define CC2520_ADCTEST1 0x57
#define CC2520_ADCTEST2 0x58
#define CC2520_FRMCTRL0 0x0c
#define CC2520_FRMCTRL1 0x0d

#define CC2520_FRMFILT0 0x00
#define CC2520_FIFOPCTRL 0x34
#define CC2520_FRMFILT1 0x01
#define CC2520_FREQCTRL 0x2e
#define CC2520_TXCTRL 0x31
#define CC2520_DACTEST2  0x5F
#define CC2520_EXTCLOCK 0x44
#define CC2520_FSMCTRL 0x35
#define CC2520_CMD_STXCAL 0x41
#define CC2520_CMD_SFLUSHRX 0x47
#define CC2520_CMD_SFLUSHTX 0x48
#define CC2520_CMD_RXBUF 0x30
#define CC2520_CMD_TXBUF 0x3A
#define CC2520_CMD_SRXON 0x42
#define CC2520_CMD_STXON 0x43
#define CC2520_CMD_SRFOFF 0x45



#define CC2520_CMD_REGISTER_READ 0x80
#define CC2520_CMD_REGISTER_WRITE 0xc0

//status byte values
#define CC2520_STATUS_XOSC32M_STABLE (1<<7)
#define CC2520_STATUS_TX_UNDERFLOW (1<<3)
#endif
#define SET_DEV_CHANNEL_CMD 0x03
#define SET_DEV_POWER_CMD 0x04
#define STOP_DEV_RF_COM_CMD 0x20
#define START_DEV_RF_COM_CMD 0x21
#define SEND_PURE_POWER 0x40
#define FLUSH_DEV_RF_DATA_CMD 0x30

static const unsigned char powerTable[8] = {
	0x03,//-1dbm
	0x49,//5dbm
	0x79,//10dbm
	0xA0,//14dbm
	0xE0,//15dbm
	0xF0,//16dbm
	0xF9,//17dbm
	0xFF,
};
struct cc2520_private {
	struct spi_device *spi;

	u8 *buf; //SPI TX/RX data buffer
	struct mutex buffer_mutex; //SPI buffer mutex
	bool is_tx; //Flag or sync b/w Tx and RX
	int fifo_pin; //FIFO GPIO pin number
	struct work_struct fifop_irqwork;//Workqueue for FIFOP
	spinlock_t lock;
	struct completion tx_complete;//Work completion for Tx
	struct sk_buff_head skb_queue;
};

struct cc2520_platform_data{
	int fifo;
	int fifop;
	int sfd;
	int tx;

	int reset;
	int vreg;

	//cc2591
	int rx_en;
	int hgm_en;
	int paen;
};

struct cc2520_platform_data *g_pdata;
struct cc2520_private *g_private_data;

static int cc2520_cmd_strobe(struct cc2520_private *priv, u8 cmd){
	int ret;
	u8 status = 0xff;
	struct spi_message msg;
	struct spi_transfer xfer = {
		.len = 0,
		.tx_buf = priv->buf,
		.rx_buf = priv->buf,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	mutex_lock(&priv->buffer_mutex);
	priv->buf[xfer.len++] = cmd;

	ret = spi_sync(priv->spi, &msg);
	if(!ret)
	  status = priv->buf[0];
	mutex_unlock(&priv->buffer_mutex);

	return ret;
}
static int cc2520_get_status(struct cc2520_private *priv, u8 *status){
	int ret;
	struct spi_message msg;
	struct spi_transfer xfer = {
		.len = 0,
		.tx_buf = priv->buf,
		.rx_buf = priv->buf,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	mutex_lock(&priv->buffer_mutex);
	priv->buf[xfer.len++] = CC2520_CMD_SNOP;
	
	ret = spi_sync(priv->spi, &msg);
	if(!ret)
	  *status = priv->buf[0];
	mutex_unlock(&priv->buffer_mutex);

	return ret;
}

static int cc2520_write_register(struct cc2520_private *priv, u8 reg, u8 value){
	int status;
	struct spi_message msg;
	struct spi_transfer xfer = {
		.len = 0,
		.tx_buf = priv->buf,
		.rx_buf = priv->buf,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	mutex_lock(&priv->buffer_mutex);
	if(reg <= CC2520_FREG_MASK){
		priv->buf[xfer.len++] = CC2520_CMD_REGISTER_WRITE | reg;
		priv->buf[xfer.len++] = value;
	}else{
		priv->buf[xfer.len++] = CC2520_CMD_MEMORY_WRITE;
		priv->buf[xfer.len++] = reg;
		priv->buf[xfer.len++] = value;
	}

	status = spi_sync(priv->spi, &msg);
	if(msg.status)
	  status = msg.status;
	mutex_unlock(&priv->buffer_mutex);

	return status;
}

static int cc2520_read_register(struct cc2520_private *priv, u8 reg, u8 *data){
	int status;
	struct spi_message msg;
	struct spi_transfer xfer1 ={
		.len = 0,
		.tx_buf = priv->buf,
		.rx_buf = priv->buf,
	};
	struct spi_transfer xfer2 = {
		.len = 1,
		.rx_buf = data,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer1, &msg);
	spi_message_add_tail(&xfer2, &msg);

	mutex_lock(&priv->buffer_mutex);
	priv->buf[xfer1.len++] = CC2520_CMD_MEMORY_READ;
	priv->buf[xfer1.len++] = reg;

	status = spi_sync(priv->spi, &msg);
	if(msg.status)
	  status = msg.status;
	mutex_unlock(&priv->buffer_mutex);
	
	return status;
}

static int cc2520_read_rxfifo(struct cc2520_private *priv, u8 *data, u8 len){
	int status;
	struct spi_message msg;

	struct spi_transfer xfer_head = {
		.len = 0,
		.tx_buf = priv->buf,
		.rx_buf = priv->buf,
	};

	struct spi_transfer xfer_buf = {
		.len = len,
		.rx_buf = data,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer_head, &msg);
	spi_message_add_tail(&xfer_buf, &msg);

	mutex_lock(&priv->buffer_mutex);
	priv->buf[xfer_head.len++] = CC2520_CMD_RXBUF;

	printk("bshui read rxfifo buf[0]=0x%x\n", priv->buf[0]);
	printk("bshui buf[1]=0x%x\n", priv->buf[1]);

	status = spi_sync(priv->spi, &msg);
	if(msg.status)
	  status = msg.status;
	printk("bshui return status buf[0]=0x%x length buf[1]=0x%x\n",
				priv->buf[0], priv->buf[1]);

	mutex_unlock(&priv->buffer_mutex);

	return status;
}

static int cc2520_rx(struct cc2520_private *priv)
{
	u8 len = 0, lqi = 0, bytes = 1,validbytes = 0,rssivalue = 0;
	struct sk_buff *skb;
	u8 recvBuff[128]={0};
	
	u8 index = 0;
	u32 queuelen = 0;
	cc2520_read_rxfifo(priv, &len, 1);

	if (len < 2 || len > 127)
		return -EINVAL;
	
	validbytes = len -2;
	if (cc2520_read_rxfifo(priv, recvBuff, len)) {
		//my_dev_dbg(&priv->spi->dev, "frame reception failed\n");
		return -EINVAL;
	}

	if((recvBuff[0] != 0x08) && (recvBuff[0] != 0x18)){//Tag  to  Reader 
		return -EINVAL;
	}

	
	rssivalue = recvBuff[len-1];

	queuelen = skb_queue_len(&g_private_data->skb_queue);
	if(queuelen < 10){//if(reader->skb_queue != NULL){
		skb = alloc_skb(len, GFP_KERNEL);
		if (!skb)
			return -ENOMEM;

		memcpy(skb_put(skb, len),recvBuff,len);
		
		printk("[cc2520_rx len= %d]\n",skb->len);

	
		skb_queue_tail(&(g_private_data->skb_queue), skb);
	
	
		
	
	}else{
		printk("reader rx irq queue len :%d\n",queuelen);
		dev_kfree_skb(skb);
		skb = NULL;}
	


	return 0;
}

static void cc2520_fifop_irqwork(struct work_struct *work){
	struct cc2520_private *priv = container_of(work, struct cc2520_private,
				fifop_irqwork);

	printk("bshui cc2520_fifop_irqwork fifop interrupt received\n");

	if(gpio_get_value(priv->fifo_pin))
	  cc2520_rx(priv);
	else
	  printk("bshui rxfifo overflow\n");

	cc2520_cmd_strobe(priv, CC2520_CMD_SFLUSHRX);
	cc2520_cmd_strobe(priv, CC2520_CMD_SFLUSHRX);
}

static irqreturn_t cc2520_fifop_isr(int irq, void *data){
	struct cc2520_private *priv = data;
	printk("bshui cc2520_fifop_isr\n");
	schedule_work(&priv->fifop_irqwork);

	return IRQ_HANDLED;
}

static irqreturn_t cc2520_sfd_isr(int irq, void *data){
	struct cc2520_private *priv = data;
	unsigned long flags;

	spin_lock_irqsave(&priv->lock, flags);
	if(priv->is_tx){
		priv->is_tx = 0;
		spin_unlock_irqrestore(&priv->lock, flags);
		//printk("bshui SFD for TX\n");
		complete(&priv->tx_complete);
	}else{
		spin_unlock_irqrestore(&priv->lock, flags);
		//printk("bshui SFD for RX\n");
	}

	return IRQ_HANDLED;
}

static void cc2520_gpio_init(struct cc2520_platform_data *pdata){
	pdata->fifo  = GPIO1_FIFO;
	pdata->fifop = GPIO2_FIFOP;
	pdata->sfd   = GPIO4_SFD;
	pdata->vreg  = GPIO_VREG_EN;
	pdata->reset = GPIO_RESET_EN;
	pdata->tx    = GPIO_TX;
	
	pdata->paen   = GPIO_2591_PAEN;
	pdata->rx_en  = GPIO_2591_RXEN;
	pdata->hgm_en = GPIO_2591_HGMEN;

	gpio_request_one(pdata->fifo, GPIOF_IN, "fifo");
//	gpio_request_one(pdata->fifop, GPIOF_IN, "fifop");
//	gpio_request_one(pdata->sfd, GPIOF_IN, "sfd");
	gpio_request_one(pdata->tx, GPIOF_OUT_INIT_LOW, "tx");
	gpio_request_one(pdata->reset, GPIOF_OUT_INIT_LOW, "reset");
	gpio_request_one(pdata->vreg, GPIOF_OUT_INIT_LOW, "vreg");
	gpio_request_one(pdata->rx_en, GPIOF_OUT_INIT_LOW, "rx_en");
	gpio_request_one(pdata->hgm_en, GPIOF_OUT_INIT_LOW, "hgm_en");
	gpio_request_one(pdata->paen, GPIOF_OUT_INIT_LOW, "paen");

	gpio_set_value(pdata->vreg, HIGH);
	usleep_range(100,150);

	gpio_set_value(pdata->reset, HIGH);
	usleep_range(200, 250);

}

static int cc2520_hw_init(struct cc2520_private *priv){
	u8 status = 0, state = 0xff;
	int ret;
	int timeout = 100;

	ret = cc2520_read_register(priv, CC2520_FSMSTAT1, &state);
	if(ret)
	  goto err_ret;
	if(state != STATE_IDLE)
	  return -EINVAL;

	do{
		ret = cc2520_get_status(priv, &status);
		if(ret)
		  goto err_ret;
		if(timeout-- <= 0){
			printk("bshui cc2520 oscillator start failed\n");
			return ret;
		}
		udelay(1);
	
	}while(!(status & CC2520_STATUS_XOSC32M_STABLE));
	printk("bshui cc2520_hw_init oscillator brought up\n");
	//Registers default value 
	cc2520_write_register(priv, CC2520_FIFOPCTRL, 0x7F);
	cc2520_write_register(priv, CC2520_FRMFILT1, 0x10);
	cc2520_write_register(priv, CC2520_TXCTRL, 0xC1);
	cc2520_write_register(priv, CC2520_CCACTRL0, 0xF8);
	cc2520_write_register(priv, CC2520_MDMCTRL0, 0x85);
	cc2520_write_register(priv, CC2520_MDMCTRL1, 0x14);
	cc2520_write_register(priv, CC2520_RXCTRL, 0x3F);
	cc2520_write_register(priv, CC2520_FSCTRL, 0x5A);
	cc2520_write_register(priv, CC2520_FSCAL1, 0x03);
	cc2520_write_register(priv, CC2520_AGCCTRL1, 0x16);
	cc2520_write_register(priv, CC2520_ADCTEST0, 0x10);
	cc2520_write_register(priv, CC2520_ADCTEST1, 0x0E);
	cc2520_write_register(priv, CC2520_ADCTEST2, 0x03);

	cc2520_write_register(priv, CC2520_EXTCLOCK, 0x00);
	cc2520_write_register(priv, CC2520_FSMCTRL, 0x00);
	cc2520_write_register(priv, CC2520_FRMFILT0, 0xFE);
	cc2520_write_register(priv, CC2520_FRMCTRL1, 0x03);
	cc2520_write_register(priv, CC2520_TXPOWER, 0xFE);
	
err_ret:
	return ret;
}


static void cc2520_start(void){
	printk("bshui cc2520_start\n");

	gpio_set_value(g_pdata->vreg, HIGH);

	gpio_set_value(g_pdata->hgm_en, HIGH);
	gpio_set_value(g_pdata->rx_en, HIGH);
	gpio_set_value(g_pdata->paen, LOW);
	
	cc2520_cmd_strobe(g_private_data, CC2520_CMD_SRXON);

}

static void cc2520_stop(void){
	printk("bshui cc2520_stop\n");

	cc2520_cmd_strobe(g_private_data, CC2520_CMD_SRFOFF);
	gpio_set_value(g_pdata->hgm_en, LOW);
	gpio_set_value(g_pdata->rx_en, LOW);
	gpio_set_value(g_pdata->paen, LOW);
}

static int cc2520_write_txfifo(struct cc2520_private *priv, u8 *data, u8 len){
	int status;
	int len_byte = len+2;

	struct spi_message msg;
	struct spi_transfer xfer_head = {
		.len = 0,
		.tx_buf = priv->buf,
		.rx_buf = priv->buf,
	};

	struct spi_transfer xfer_len = {
		.len = 1,
		.tx_buf = &len_byte,
	};

	struct spi_transfer xfer_buf = {
		.len = len,
		.tx_buf = data,
	};

	spi_message_init(&msg);
	spi_message_add_tail(&xfer_head, &msg);
	spi_message_add_tail(&xfer_len, &msg);
	spi_message_add_tail(&xfer_buf, &msg);

	mutex_lock(&priv->buffer_mutex);
	priv->buf[xfer_head.len++] = CC2520_CMD_TXBUF;

	printk("bshui TX_FIFO cmd buf[0] =0x%x\n", priv->buf[0]);

	status = spi_sync(priv->spi, &msg);
	if(msg.status)
	  status = msg.status;

	mutex_unlock(&priv->buffer_mutex);

	return status;
}

static int cc2520_tx(struct sk_buff *skb){
	unsigned long  flags;
	int rc;
	u8 status = 0;

	printk("bshui cc2520_tx\n");
	rc = cc2520_cmd_strobe(g_private_data, CC2520_CMD_SFLUSHTX);
	if(rc)
	  goto err_tx;

	rc = cc2520_write_txfifo(g_private_data, skb->data, skb->len);
	if(rc)
	  goto err_tx;

	cc2520_get_status(g_private_data, &status);
	if(status & CC2520_STATUS_TX_UNDERFLOW){
		printk("bshui cc2520 tx underflow exception\n");
		goto err_tx;
	}

	spin_lock_irqsave(&g_private_data->lock, flags);
	BUG_ON(g_private_data->is_tx);
	g_private_data->is_tx = 1;
	spin_unlock_irqrestore(&g_private_data->lock, flags);

	gpio_set_value(g_pdata->rx_en, LOW);
	gpio_set_value(g_pdata->paen, HIGH);

	rc = cc2520_cmd_strobe(g_private_data, CC2520_CMD_STXON);
	if(rc < 0)
	  goto err;
	
	rc = wait_for_completion_interruptible_timeout(&g_private_data->tx_complete, msecs_to_jiffies(100));
	if(rc < 0)
	  goto err;

	gpio_set_value(g_pdata->rx_en, HIGH);
	gpio_set_value(g_pdata->paen, LOW);

	cc2520_cmd_strobe(g_private_data, CC2520_CMD_SFLUSHTX);
	cc2520_cmd_strobe(g_private_data, CC2520_CMD_SRXON);

	return rc;

err:
	spin_lock_irqsave(&g_private_data->lock, flags);
	g_private_data->is_tx = 0;
	spin_unlock_irqrestore(&g_private_data->lock, flags);
err_tx:
	return rc;
}

static int cc2520_set_channel(unsigned char channel){
	int ret;
	unsigned char cnl = 0;
	if(channel < 16){
		cnl = channel+11;
		ret = cc2520_write_register(g_private_data, CC2520_FREQCTRL,
					(11+5*(cnl-11)));
	}else{
		ret = -1;
	}
	printk("bshui set_channel:%d\n", channel);
	return ret;
}

static int cc2520_set_power(unsigned char power){
	int ret;
	unsigned char pwr = 0;
	if(power < 8){
		pwr = powerTable[power];
		ret = cc2520_write_register(g_private_data, CC2520_TXPOWER, pwr);
	}else{
		ret = -1;
	}
	printk("bshui set_power:%d\n", power);
	return 0;
}


static int send_purewave(u8 channel, u8 power){
	printk("bshui cc2520 send purewave\n");

	cc2520_cmd_strobe(g_private_data, CC2520_CMD_SRFOFF);
	cc2520_set_channel(channel);
	cc2520_set_power(power);

	cc2520_write_register(g_private_data, CC2520_FRMCTRL0, 0x43);
	cc2520_write_register(g_private_data, CC2520_DACTEST2, 0x29);
	cc2520_cmd_strobe(g_private_data, CC2520_CMD_STXCAL);
	printk("bshui g_pdata=%d\n", g_pdata->vreg);

	gpio_set_value(g_pdata->rx_en, LOW);
	gpio_set_value(g_pdata->paen, HIGH);

	cc2520_cmd_strobe(g_private_data, CC2520_CMD_STXON);

	return 0;
}

static long reader_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	unsigned char *pdata = (unsigned char *)arg;
	int len = (cmd &0x0000ff00)>>8;

	printk("bshui reader ioctl cmd=%d data=%d\n", cmd, *pdata);
	switch(cmd){
		case SET_DEV_CHANNEL_CMD:
			cc2520_set_channel(pdata);
			break;
		case SET_DEV_POWER_CMD:
			cc2520_set_power(*pdata);
			break;
		case START_DEV_RF_COM_CMD:
			cc2520_start();
			break;
		case STOP_DEV_RF_COM_CMD:
			cc2520_stop();
			break;
		case FLUSH_DEV_RF_DATA_CMD:
			skb_queue_purge(&g_private_data->skb_queue);
			break;
		case SEND_PURE_POWER:
			send_purewave(*pdata, *(pdata+1));
			break;
		default:
			break;
	}

	return 0;
}

static ssize_t reader_read(struct file *file, char __user *data, size_t count, loff_t *f_pos){
	struct sk_buff *skb = NULL;
	int len=0;
	printk("bshui reader_read\n");

	if((skb = skb_dequeue(&g_private_data->skb_queue))!= NULL){
		printk("bshui reader_read rx len=%d\n", skb->len);
		copy_to_user(data, skb->data, skb->len);
		len = skb->len;
		dev_kfree_skb(skb);
		skb = NULL;
	}else{
		printk("bshui reader rx data is null\n");
	}



	return len;
}

static ssize_t reader_write(struct file *file, const char __user *data, size_t count, loff_t *f_pos){
	struct sk_buff *skb;
	unsigned long ret;
	printk("bshui reader_write\n");

	if(count <=0 || data == NULL)
	  return -1;

	skb = alloc_skb(count, GFP_KERNEL);
	if(!skb)
	  return -ENOMEM;

	ret = copy_from_user(skb_put(skb, count),data, count);

	cc2520_tx(skb);

	dev_kfree_skb(skb);
	skb = NULL;


	return count;
}

static int reader_open(struct inode *inode, struct file *file){
	cc2520_start();
	return 0;
}

static int reader_release(struct inode *inode, struct file *file){
	cc2520_stop();
	return 0;
}

static const struct file_operations reader_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = reader_ioctl,
	.read			= reader_read,
	.write			= reader_write,
	.open			= reader_open,
	.release		= reader_release,
};

static struct miscdevice reader_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	//.name = "2P45Reader",
	.name = "2.4GReader",
	.fops = &reader_fops,
};
static int cc2520_probe(struct spi_device *spi){
	struct cc2520_private *priv;
	struct cc2520_platform_data *pdata;
	int ret = 0;

	priv = devm_kzalloc(&spi->dev, sizeof(struct cc2520_private), GFP_KERNEL);

	spi_set_drvdata(spi, priv);

	priv->spi = spi;
	priv->buf = devm_kzalloc(&spi->dev, SPI_COMMAND_BUFFER, GFP_KERNEL);

	skb_queue_head_init(&priv->skb_queue);

	pdata = devm_kzalloc(&spi->dev, sizeof(struct cc2520_platform_data), GFP_KERNEL);

	mutex_init(&priv->buffer_mutex);
	spin_lock_init(&priv->lock);
	init_completion(&priv->tx_complete);
	INIT_WORK(&priv->fifop_irqwork, cc2520_fifop_irqwork);

	cc2520_gpio_init(pdata);
	g_pdata = pdata;
	priv->fifo_pin = pdata->fifo; 
	
	g_private_data = priv;
	cc2520_hw_init(priv);


	//set up fifop interrupt
	ret = devm_request_threaded_irq(&spi->dev, gpio_to_irq(pdata->fifop),NULL, cc2520_fifop_isr,IRQF_TRIGGER_RISING,dev_name(&spi->dev),priv);
	if(ret){
		printk("bshui could not get fifop irq\n");
	}

	//set up sfd interrupt
	devm_request_threaded_irq(&spi->dev, gpio_to_irq(pdata->sfd),NULL,cc2520_sfd_isr,IRQF_TRIGGER_FALLING,dev_name(&spi->dev),priv);

	misc_register(&reader_miscdev);

	printk("bshui cc2520_probe---------ok----\n");
	return 0;
}

static int cc2520_remove(struct spi_device *spi){
	struct cc2520_private *priv = spi_get_drvdata(spi);

	mutex_destroy(&priv->buffer_mutex);
	flush_work(&priv->fifop_irqwork);
	devm_free_irq(&spi->dev, gpio_to_irq(g_pdata->fifop), priv);
	devm_free_irq(&spi->dev, gpio_to_irq(g_pdata->sfd), priv);

	gpio_free(g_pdata->fifo);
	gpio_free(g_pdata->fifop);
	gpio_free(g_pdata->tx);
	gpio_free(g_pdata->sfd);
	gpio_free(g_pdata->vreg);
	gpio_free(g_pdata->reset);
	gpio_free(g_pdata->rx_en);
	gpio_free(g_pdata->hgm_en);
	gpio_free(g_pdata->paen);
	misc_deregister(&reader_miscdev);
	return 0;
}

static struct spi_driver cc2520_driver = {
	.driver = {
		.name = "2.4GMOD",
		.owner = THIS_MODULE,
	},
	.probe = cc2520_probe,
	.remove = cc2520_remove,
};

module_spi_driver(cc2520_driver);
MODULE_LICENSE("GPL");
