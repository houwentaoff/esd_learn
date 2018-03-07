#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
struct class *char_cdev_class;

static char dev_rw_buff[64]; //设备内部读写缓冲区

static int char_cdev_rw_open(struct inode *inode, struct file *filp){
	printk("char cdev open in kernel\n");
	return 0;
}

static int char_cdev_rw_release(struct inode *inode, struct file *file){
	return 0;
}

static ssize_t char_cdev_rw_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos){
	if(count > 64){
		printk("Max lenght is 64\n");
		count = 64;
	}

	if(copy_to_user((void*)buf, dev_rw_buff, count)){
		printk("copy_to_user err\n");
		return -EFAULT;
	}

	return count;
}

static ssize_t char_cdev_rw_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos){
		if(count > 64){
			printk("Max length is 64\n");
			count = 64;
		}

		if(copy_from_user(&dev_rw_buff, buf, count)){
			printk("copy_from_user err\n");
			return -EFAULT;
		}

		return count;
}


struct file_operations char_cdev_fops = {
	.owner = THIS_MODULE,
	.open = char_cdev_rw_open,
	.release = char_cdev_rw_release,
	.read = char_cdev_rw_read,
	.write = char_cdev_rw_write,
};

static int char_cdev_rw_init(void){
	int ret;
	int i;
	ret = register_chrdev(300, "hello", &char_cdev_fops);
	if(ret < 0){
		printk("unable to register character device \n");
		return ret;
	}
	char_cdev_class = class_create(THIS_MODULE, "char_cdev");
	if(IS_ERR(char_cdev_class)){
		printk("class create fail\n");
	}
	device_create(char_cdev_class, NULL, MKDEV(300,0),NULL, "char_cdev");

	//init the dat buf
	for(i=0; i<64; i++)
		dev_rw_buff[i] = i;

	return 0;
}

static void char_cdev_rw_exit(void){
	device_destroy(char_cdev_class, MKDEV(300,0));
	class_destroy(char_cdev_class);
	unregister_chrdev(300,"hello");
}

module_init(char_cdev_rw_init);
module_exit(char_cdev_rw_exit);
MODULE_LICENSE("Dual BSD/GPL");
