# Very Basic kernel module builder
# assumes that the kernel is setup to provide a basic build environment
#
KERNEL_DIR=/lib/modules/$(shell uname -r)/build
obj-m += my_mod.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean