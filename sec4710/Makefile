ARCH=arm
CROSS_COMPILE=/home/jz/tools/arm-2007q3/bin/arm-none-linux-gnueabi-
obj-m += se4500.o
KDIR := /home/jz/nfs_artillery/kernel_imx/
#KDIR := /home/jz/nfs_server/kernel_imx/
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
