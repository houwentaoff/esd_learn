ARCH=arm
CROSS_COMPILE=/home/jz/tools/arm-2007q3/bin/arm-none-linux-gnueabi-
obj-m += jp1081_android.o
KDIR := /home/jz/myandroid/kernel_imx/
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
