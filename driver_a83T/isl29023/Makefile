ARCH=arm
CROSS_COMPILE=/home/a83t/l_a83t/lichee/brandy/gcc-linaro/bin/arm-linux-gnueabi-
obj-m += isl29023_delay.o
KDIR := /home/a83t/l_a83t/lichee/linux-3.4/
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
