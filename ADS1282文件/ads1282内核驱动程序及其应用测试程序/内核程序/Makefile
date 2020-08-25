export ARCH=arm 
export CROSS_COMPILE=arm-linux-
obj-m :=ads1282_kernel.o 
PWD=$(shell pwd)
KDIR :=/usr/local/arm/linux_E9_3.0.35_for_Linux
all:
	make -C	$(KDIR)	M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers
