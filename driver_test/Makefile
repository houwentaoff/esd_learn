obj-m += led_ioctl.o
KDIR := /lib/modules/$(shell uname -r)/build
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
