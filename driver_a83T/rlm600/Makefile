ARCH=arm
CROSS_COMPILE=/home/jz/tools/gcc-linaro/bin/arm-linux-gnueabi-
obj-m += gpio_uhf.o
KDIR := /home/jz/test/linux-3.4.A83t/
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
