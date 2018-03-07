/*************************************************************************
	> File Name: virtualnetdevice.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月19日 星期日 14时59分55秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

MODULE_LICENSE("GPL");
static struct net_device *virtualnetdevice_card;
#define VIRTUALNETDEVICE_CARD_NAME "usb0"

static int virtualnetdevice_open(struct net_device *dev){
	printk("virtualnetdevice open\n");
	return 0;
}

static int virtualnetdevice_stop(struct net_device *dev){
	printk("virtualdevice stop\n");

	return 0;
}

static int virtualnetdevice_hard_start_xmit(struct sk_buff *skb,
		struct net_device *dev){
	printk("entry virtualnetdevice_hard_start_xmit\n");

	dev_kfree_skb(skb);
	return 0;
}

static int virtualnetdevice_card_init(struct net_device *dev){
	printk("entry virtualnetdevice_card_init\n");

	ether_setup(dev);
	strcpy(dev->name, VIRTUALNETDEVICE_CARD_NAME);

	return 0;
}


static const struct net_device_ops virt_netdev_ops = {
	.ndo_init = virtualnetdevice_card_init,
	.ndo_open = virtualnetdevice_open,
	.ndo_stop = virtualnetdevice_stop,
	.ndo_start_xmit = virtualnetdevice_hard_start_xmit,
};

static int __init virtualnetdevice_init(void){
	int ret = 0;
	printk("entry virtualnetdevice_init\n");
	
	virtualnetdevice_card = alloc_etherdev(0);
	virtualnetdevice_card->netdev_ops = &virt_netdev_ops;
	virtualnetdevice_card->dev_addr[0] = 0x08;
	virtualnetdevice_card->dev_addr[1] = 0x09;
	virtualnetdevice_card->dev_addr[2] = 0x0a;
	virtualnetdevice_card->dev_addr[3] = 0x0b;
	virtualnetdevice_card->dev_addr[4] = 0x0c;
	virtualnetdevice_card->dev_addr[5] = 0x0d;

	//register net_device
	
	ret = register_netdev(virtualnetdevice_card);
	if(ret < 0){
		printk("fail to register net device\n");
		goto failure_register_netdev;
	}
	printk("success to register net device %s\n", VIRTUALNETDEVICE_CARD_NAME);

	return ret;
failure_register_netdev:
	return ret;

}

static void __exit virtualnetdevice_exit(void){
	unregister_netdev(virtualnetdevice_card);
}

module_init(virtualnetdevice_init);
module_exit(virtualnetdevice_exit);
