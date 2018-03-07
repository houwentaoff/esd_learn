#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

struct class *hello_class;

unsigned int hello_irq = 10;

int hello_open(struct inode *inode, struct file *filp){

	return 0;
}

static ssize_t hello_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos){

	return 0;
}
static ssize_t hello_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos){
	return 0;
}

static long hello_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){

	return 0;
}


static int hello_release(struct inode *inode, struct file *filp){
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

static void hello_tasklet_func(unsigned long data){
	printk("entry hello tasklet_func\n");

}

DECLARE_TASKLET(hello_tasklet, hello_tasklet_func, 0);

static irqreturn_t hello_isr(int irq, void *dev_id){
	printk("entry driver_isr\n");
	
	tasklet_schedule(&hello_tasklet);

	return IRQ_HANDLED;
}

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


	ret = request_irq(hello_irq, hello_isr, 0, "interruptdemo", NULL);
	if(ret < 0){
		printk("failure to request irq 0x%x\n", hello_irq);
		goto failure_request_irq;
	}
	printk("success to request irq\n");
	return 0;
failure_request_irq:
	return -1;
}

static void hello_exit(void){
	device_destroy(hello_class, MKDEV(300,0));
	class_destroy(hello_class);
	unregister_chrdev(300,"hello");
	free_irq(hello_irq, NULL);
	printk("hello_exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("Dual BSD/GPL");
