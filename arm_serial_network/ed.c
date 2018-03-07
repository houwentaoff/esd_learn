#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>

#include "ed_device.h"
#include "ed_ioctl.h"
#define _DEBUG
struct ed_device ed[2];
static struct class *ed_rec_dev_class;
static struct class *ed_tx_dev_class;
static int timeout = ED_TIMEOUT;
struct net_device *ednet_dev;
/*read data from ed_tx device*/
static ssize_t device_read(struct file *file,char __user *buffer,
		size_t length, loff_t *ppos)
{
	printk("-----%s--------\n", __func__);
	#ifdef _DEBUG
	int i;
	#endif
	struct ed_device *edp;
	edp = (struct ed_device *)file->private_data;
	DECLARE_WAITQUEUE(wait, current);
	add_wait_queue(&edp->rwait, &wait);
	for(; ;){
		set_current_state(TASK_INTERRUPTIBLE);
		if( file->f_flags & O_NONBLOCK)
		{
			printk("break 1\n");
			break;
		}
		if(signal_pending(current)){
			printk("break 2\n");
			break;
		}
		printk("-------a---------\n");
		schedule();
		printk("-----------b---------\n");
	}
	set_current_state(TASK_RUNNING);
	remove_wait_queue(&edp->rwait, &wait);
	printk("--------3----------\n");
	spin_lock(&edp->lock);
	if(edp->tx_len == 0){
		printk("sorry return the tx_len = %d\n", edp->tx_len);
		spin_unlock(&edp->lock);
		return 0;
	}else{
		copy_to_user(buffer, edp->buffer, edp->tx_len);
		memset(edp->buffer, 0, edp->buffer_size);
		printk("----------4-----------\n");
		#ifdef _DEBUG
		printk("\n read data from ed_tx \n");
		for(i=0; i<edp->tx_len; i++)
			printk(" %02x", edp->buffer[i]&0xff);
		printk("\n");
		#endif
		length = edp->tx_len;
		edp->tx_len = 0;
	}
	spin_unlock(&edp->lock);
	return length;
}
/*ednet_rx,recives a network packet and put the
 packet into TCP/IP up layer, netif_rx() is the kernel
 API to do such thing. the recieving procedure must 
 alloc the sk_buff structure to store the data and 
 the sk_buff will be freed in the up layer*/
void ednet_rx(struct net_device *dev, int len, 
		unsigned char *buf){
	printk("-----%s--------\n", __func__);

	struct sk_buff *skb;
	struct ednet_priv *priv = netdev_priv(dev);
	/*分配新的套接字缓冲区*/
	skb = dev_alloc_skb(len+2);
	if(!skb){
		printk("ednet_rx can not allocate more memory to\
				store the packet. drop the packet\n");
		dev->stats.rx_dropped++;
		return;
	}
	/*对齐*/
	skb_reserve(skb, 2);
	memcpy(skb_put(skb, len), buf, len);
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	dev->stats.rx_packets++;

	netif_rx(skb);

	return;
}
/*Device write is called by server program,to put the
 user space network data into ed_rec device*/
static ssize_t device_write(struct file *file, 
		const char __user *buffer, size_t length,
		loff_t *offset){
	#ifdef _DEBUG
	int i;
	#endif
printk("-----%s--------\n", __func__);

	struct ed_device *edp;
	edp = (struct ed_device *)file->private_data;
	
	spin_lock(&ed[ED_REC_DEVICE].lock);
	if(length > edp->buffer_size)
		length = edp->buffer_size;

	copy_from_user(ed[ED_REC_DEVICE].buffer, buffer, length);
	if(ednet_dev == NULL)
		return;
	else
	ednet_rx(ednet_dev, length, ed[ED_REC_DEVICE].buffer);
	#ifdef _DEBUG
	printk("\n Network Device Recive buffer:\n");
	for(i=0; i<length; i++)
		printk(" %02x",ed[ED_REC_DEVICE].buffer[i]&0xff);
		printk("\n");
	#endif

	spin_unlock(&ed[ED_REC_DEVICE].lock);
	return length;
}

static int device_ioctl(struct file *filp,
		unsigned int cmd, unsigned int arg){
	struct ed_device *edp;
printk("-----%s--------\n", __func__);

	edp = (struct ed_device *)filp->private_data;
	switch(cmd){
		case IOCTL_SET_BUSY:
			edp->busy = arg;
			break;
	}
	return 0;
}
static int device_open(struct inode *inode, 
		struct file *filp){
	int Device_Major = 0;
	struct ed_device *edp;
	printk("-----%s--------\n", __func__);
	printk("iminor(inode)=%d\n", imajor(inode));
	if(imajor(inode) == 200){
		filp->private_data = &ed[ED_REC_DEVICE];
		ed[ED_REC_DEVICE].file = filp;
	}else if(imajor(inode) == 201){
		filp->private_data = &ed[ED_TX_DEVICE];
		ed[ED_TX_DEVICE].file = filp;
	}else
		return -ENODEV;

	edp = (struct ed_device *)filp->private_data;

	if(edp->busy != 0){
		printk("The device is open\n");
		return -EBUSY;
	}

	edp->busy++;

	return 0;
}

/*release the devices*/
static int device_release(struct inode *inode,
		struct file *filp){
	printk("-----%s--------\n", __func__);

	struct ed_device *edp;
	edp = (struct ed_device *)filp->private_data;
	edp->busy = 0;

	return 0;
}
static const struct file_operations ed_ops = {
	.owner = THIS_MODULE,
	.read  = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open  = device_open,
	.release = device_release,
};
/*This function is called by ednet device to write
 the network data into the ed_tx charactr device*/
ssize_t kernel_write(const char *buffer, size_t length,
		int buffer_size){
	printk("-----%s--------\n", __func__);

	if(length > buffer_size)
		length = buffer_size;
	memset(ed[ED_TX_DEVICE].buffer, 0, buffer_size);
	memcpy(ed[ED_TX_DEVICE].buffer, buffer, buffer_size);
	ed[ED_TX_DEVICE].tx_len = length;

	wake_up_interruptible(&ed[ED_TX_DEVICE].rwait);

	return length;
}
/*
 Initialize the ed_rec and ed_tx device,the two
 devices are allocate the initial buffer tho store
 the incoming and outgoing data.
 */
int device_init(void){
	int err;
	int i;
	err = -ENOBUFS;
	
	strcpy(ed[ED_REC_DEVICE].name, ED_REC_DEVICE_NAME);
	strcpy(ed[ED_TX_DEVICE].name, ED_TX_DEVICE_NAME);
	for(i=0; i<2; i++){
		ed[i].buffer_size = BUFFER_SIZE;
		ed[i].buffer = kmalloc(ed[i].buffer_size +4, GFP_KERNEL);
		ed[i].magic = ED_MAGIC;
		ed[i].mtu = ED_MTU;
		ed[i].busy = 0;
		init_waitqueue_head(&ed[i].rwait);
		
		if(ed[i].buffer == NULL)
			goto err_exit;
		spin_lock_init(&ed[i].lock);
	}

	err = 0;
	return err;

err_exit:
	printk("There is no enongh memory for buffer allocation\n");
	return err;
}

/*Initialize the character devices*/
int eddev_module_init(void){
	int err;
	int i;

	if((err = device_init()) != 0){
		printk("Init device error:\n");
		return err;
	}
	
	err = register_chrdev(200, "ed_rec", &ed_ops);
	if(err <0) {
			printk("Install the buffer rec device %s fail\n",
				ED_REC_DEVICE_NAME);
		}
	ed_rec_dev_class = class_create(THIS_MODULE, "ed_rec");

	if(IS_ERR(ed_rec_dev_class)){
		printk("ed_rec_dev_class create fail\n");
	}
	device_create(ed_rec_dev_class, NULL,MKDEV(200,0),NULL, "ed_rec");
#if 1
	if(register_chrdev(201, "ed_tx",
				&ed_ops)){
		printk("Install the buffer tx device %s fail\n",
				ED_TX_DEVICE_NAME);
		
	}
	ed_tx_dev_class = class_create(THIS_MODULE,"ed_tx");
	if(IS_ERR(ed_tx_dev_class))
		printk("ed_tx_dev_class create fail\n");
	device_create(ed_tx_dev_class, NULL, MKDEV(201, 0),
			NULL,"ed_tx");
#endif
	for(i=0; i<2; i++)
		ed[i].kernel_write = kernel_write;
	return err;
}

void eddev_module_cleanup(void){
	int i;
	for(i=0; i<2; i++)
		kfree(ed[i].buffer);

	device_destroy(ed_rec_dev_class, MKDEV(200, 0));
	class_destroy(ed_rec_dev_class);
	unregister_chrdev(200, "ed_rec");
#if 1
	device_destroy(ed_tx_dev_class, MKDEV(201, 0));
	class_destroy(ed_tx_dev_class);
    unregister_chrdev(201, "ed_tx");
#endif
}

static int ednet_open(struct net_device *dev){
	memcpy(dev->dev_addr, "\0ED000", ETH_ALEN);
	netif_start_queue(dev);
	return 0;
}
static int ednet_release(struct net_device *dev){
	netif_stop_queue(dev);
	return 0;
}

static void ednet_tx_timeout(struct net_device *ndev){
	struct ednet_priv *priv = netdev_priv(ndev);
	ndev->stats.tx_errors++;
	netif_wake_queue(ndev);
}
static void ednet_hw_tx(char *buf, int len, struct net_device *dev){
	struct ednet_priv *priv = netdev_priv(dev);
	/*检查IP包的长度，大小34*/
	if(len < sizeof(struct ethhdr) + sizeof(struct iphdr)){
		printk("Bad packet! It's size is less then 34!\n");
		return;
	}
	printk("------%s------buffer_size=%d\n", __func__, ed[ED_TX_DEVICE].buffer_size);
	/*now push the data into ed_tx device*/
	ed[ED_TX_DEVICE].kernel_write(buf, len, ed[ED_TX_DEVICE].buffer_size);
	dev->stats.tx_packets++;
	dev->stats.rx_bytes +=len;
	dev_kfree_skb(priv->skb);
}
static int ednet_tx(struct sk_buff *skb,
		struct net_device *dev){
	printk("-------%s-----------\n", __func__);
	int len;
	char *data;
	struct ednet_priv *priv = netdev_priv(dev);
	if(ed[ED_TX_DEVICE].busy == 1){
		printk("the device busy\n");
		return -EBUSY;
	}
	if(skb == NULL){
		printk("------skb=NULL\n");
		ednet_tx_timeout(dev);
		if(skb == NULL)
			return 0;
	}

	len = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;
	data = skb->data;
	//记录发送时间戮
	dev->trans_start = jiffies;
	priv->skb = skb;
	/*虚拟网卡传输数据包*/
	ednet_hw_tx(data, len, dev);

	return 0;
}
/*
 Used by ifconfig,the io base addr and IRQ can be
 modified when the net device is not running
 */
static int ednet_config(struct net_device *dev,
		struct ifmap *map){
	if(dev->flags & IFF_UP)
		return -EBUSY;
	/*change the io_base addr*/
	if(map->base_addr != dev->base_addr){
		printk(KERN_WARNING "ednet:Can't change I/O address\n");
		return -EOPNOTSUPP;
	}
	/*can change the irq*/
	if(map->irq != dev->irq)
		dev->irq = map->irq;

	return 0;
}
/*when we need some ioctls*/
static int ednet_ioctl(struct net_deivce *ndev, 
		struct ifreq *rq, int cmd){

	return 0;
}
static int ed_realloc(int new_mtu){
	int err;
	int i;
	err = -ENOBUFS;
	char *local_buffer[2];
	int size;
	for(i=0; i<2; i++){
		local_buffer[i] = kmalloc(new_mtu + 4, GFP_KERNEL);
		size = min(new_mtu, ed[i].buffer_size);
		memcpy(local_buffer[i], ed[i].buffer, size);
		kfree(ed[i].buffer);

		ed[i].buffer = kmalloc(new_mtu + 4, GFP_KERNEL);
		if(ed[i].buffer < 0){
			printk("Can not realloc the buffer from kernel when change mtu\n");
			return err;
		}
	}
	return 0;
}
/*set new MTU size*/
int ednet_change_mtu(struct net_device *dev, int new_mtu){
	struct ednet_priv *priv = netdev_priv(dev);
	int err;
	unsigned long flags;
	spinlock_t *lock = &priv->lock;
	if(new_mtu < 68)
		return -EINVAL;
	spin_lock_irqsave(lock, flags);
	dev->mtu = new_mtu;
	/*realloc the new buffer*/
	err = ed_realloc(new_mtu);
	spin_unlock_irqrestore(lock, flags);

	return err;
}

static const struct net_device_ops ednet_ops = {
	.ndo_open = ednet_open,
	.ndo_stop = ednet_release,
	.ndo_start_xmit = ednet_tx,
	.ndo_tx_timeout = ednet_tx_timeout,
	.ndo_set_config = ednet_config,
	.ndo_do_ioctl = ednet_ioctl,
	.ndo_change_mtu = ednet_change_mtu,
};
static void ednet_init(struct net_device *dev){
	struct ednet_priv *ednet = netdev_priv(dev);
	dev->netdev_ops = &ednet_ops;
	dev->flags |= IFF_NOARP;
	spin_lock_init(&ednet->lock);
}
/*Initialize the net devices*/
int ednet_module_init(void){
	int err;
	struct ednet_priv *ednet;
	ednet_dev = alloc_netdev(sizeof(struct ednet_priv), "ed0", ednet_init);
	if(!ednet_dev)
		return -ENOMEM;

	err = register_netdev(ednet_dev);
	if(err < 0)
		printk("ednet: error %i registering pseudo network [%s]\n",
				err, "ed0");	
	return err;
}

void ednet_module_cleanup(void){
	unregister_netdev(ednet_dev);
	free_netdev(ednet_dev);
}



static void __exit vndev_module_cleanup(void){
	printk("[virtual ethernet Driver cleanup]\n");
	eddev_module_cleanup();
	ednet_module_cleanup();
}

static int __init vndev_module_init(void){
	int err;
	printk("[virtual ethernet Driver]\n");

	err = eddev_module_init();
	if(err < 0)
		return err;
	err = ednet_module_init();
	if(err < 0)
		return err;

	return err;
}
module_exit(vndev_module_cleanup);
module_init(vndev_module_init);
MODULE_DESCRIPTION("Support for the virtual ethernet card");
MODULE_LICENSE("GPL");
