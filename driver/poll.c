#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
struct class *hello_class;

int counts = 1;
wait_queue_head_t wqh;

int hello_open(struct inode *inode, struct file *filp){

	return 0;
}

static ssize_t hello_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos){

	wait_event_interruptible(wqh, 0!=counts);
	counts = 0;
	return 0;
}
static ssize_t hello_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos){
	counts = 1;
	wake_up_interruptible(&wqh);
	return 0;
}

static long hello_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){

	return 0;
}
static int hello_release(struct inode *inode, struct file *filp){
	return 0;
}

static unsigned int hello_poll(struct file *filp, struct poll_table_struct *wait){
	unsigned int mask = 0;
	poll_wait(filp, &wqh, wait );
	if(counts != 0){
		mask |= POLLIN | POLLRDNORM;
		printk("device is ready for reading mask:=0x%4x\n", mask);
		
	}else{
		printk("device is not ready for reading\n");
	}
	return mask;
}
struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.read = hello_read,
	.write = hello_write,
	.unlocked_ioctl = hello_ioctl,
	.release = hello_release,
	.poll = hello_poll,
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
	init_waitqueue_head(&wqh);
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
