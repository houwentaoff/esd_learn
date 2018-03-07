#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
struct class *hello_class;

struct fasync_struct *async_queue;

int hello_open(struct inode *inode, struct file *filp){

	return 0;
}

static ssize_t hello_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos){

	return 0;
}
static ssize_t hello_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos){
	if(async_queue){
		printk("device is ready for reading, and release SIGIO signal\n");
		kill_fasync(&async_queue, SIGIO, POLL_IN);
	}
	return 0;
}

static long hello_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){

	return 0;
}

static int hello_fasync(int fd, struct file *filp, int mode){
	printk("entery driver fasync\n");
	return fasync_helper(fd, filp, mode, &async_queue);
}

static int hello_release(struct inode *inode, struct file *filp){
	hello_fasync(-1, filp, 0);
	return 0;
}
struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.read = hello_read,
	.write = hello_write,
	.unlocked_ioctl = hello_ioctl,
	.release = hello_release,
	.fasync = hello_fasync,
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
