#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
struct class *hello_class;
#define ALLOW_OPEN_TIMES 2
spinlock_t lock;
int open_times;

int hello_open(struct inode *inode, struct file *filp){
	spin_lock(&lock);
	if(ALLOW_OPEN_TIMES <= open_times){
		spin_unlock(&lock);
		printk("device can not be open over %d times\n", ALLOW_OPEN_TIMES);
		return -EBUSY;
	}
	open_times++;
	spin_unlock(&lock);
	return 0;
}

static int hello_release(struct inode *inode, struct file *filp){
	spin_lock(&lock);
	open_times--;
	spin_unlock(&lock);
	return 0;
}
struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.release = hello_release,
};

static int hello_init(void){
	int ret;
	ret = register_chrdev(300, "hello", &hello_fops);
	if(ret < 0){
		printk("unable to register character device \n");
		return ret;
	}
	hello_class = class_create(THIS_MODULE, "hello");
	if(IS_ERR(hello_class)){
		printk("hello_class create fail\n");
	}
	device_create(hello_class, NULL, MKDEV(300,0),NULL, "hello");
	printk("hello_init\n");
	//initialize spinlock_t varible
	spin_lock_init(&lock);
	open_times = 0;
	return 0;
}

static void hello_exit(void){
	device_destroy(hello_class, MKDEV(300,0));
	class_destroy(hello_class);
	unregister_chrdev(300,"hello");
	printk("hello_exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("Dual BSD/GPL");
