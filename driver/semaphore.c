#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
struct class *hello_class;

struct semaphore sem_open;
struct semaphore sem_read;
struct semaphore sem_write;

int hello_open(struct inode *inode, struct file *filp){
	//try to get semaphore sem_open
	if(down_trylock(&sem_open)){
		printk("device can be open only 2 times");
		return -EBUSY;
	}
	printk("success to get semaphore sem_open\n");
	return 0;
}

static ssize_t hello_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos){
	//try to get semaphore sem_read
	down_interruptible(&sem_read);
	//critical section code
	printk("success to get semaphore sem_read\n");

	return 0;
}
static ssize_t hello_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos){
	if(down_trylock(&sem_write)){
		printk("device is waiting for something\n");
		return -EBUSY;
	}
	printk("Success to get semaphore sem_write\n");

	return 0;
}

static long hello_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	up(&sem_read);
	up(&sem_write);

	return 0;
}
static int hello_release(struct inode *inode, struct file *filp){
	up(&sem_open);
	return 0;
}
struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.read = hello_read,
	.write = hello_write,
	.unlocked_ioctl = hello_ioctl,
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
	//initialize semaphore varible
	sema_init(&sem_open, 2);
	sema_init(&sem_read, 1);
	sema_init(&sem_write, 1);
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
