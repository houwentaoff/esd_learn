ARCH=arm
CROSS_COMPILE=/home/jz/nfs_bshui/l_a83t/lichee/brandy/gcc-linaro/bin/arm-linux-gnueabi-
obj-m += cc2520.o
KDIR := /home/jz/nfs_bshui/l_a83t/lichee/linux-3.4/
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
