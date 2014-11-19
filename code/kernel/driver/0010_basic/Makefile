# Very Basic kernel module builder
# assumes that the kernel is setup to provide a basic build environment
#
KERNEL_DIR=/lib/modules/$(shell uname -r)/build
MY_MODULE = my_driver_01.o my_driver_02.o my_driver_03.o my_driver_03.o
MY_MODULE += my_driver_04.o my_driver_05.o

obj-m += $(MY_MODULE)

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean