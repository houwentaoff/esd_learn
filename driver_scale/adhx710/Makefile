ARCH=arm
CROSS_COMPILE=/home/bshui/sm9300/fsl-linaro-toolchain-master/bin/arm-fsl-linux-gnueabi-
obj-m += ad_hx710.o
KDIR := /home/freescale/m_scale_smarc/kernel_imx/
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
