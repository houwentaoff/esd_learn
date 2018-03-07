ARCH=arm
CROSS_COMPILE=/home/a83t/Vehicle_Mounted_A83t/lichee/brandy/gcc-linaro/bin/arm-linux-gnueabi-
obj-m += mag3110.o
KDIR := /home/a83t/Vehicle_Mounted_A83t/lichee/linux-3.4/
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
