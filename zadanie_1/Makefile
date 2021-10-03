obj-m := chardev.o
chardev-objs := chardev_debugfs.o chardev_main.o chardev_sysfs.o

all:
	make -C $(KDIR) M=$(shell pwd) modules
clean:
	@-rm -rf *.mod *.order *.symvers *.o *.mod.* *.dwo *.ko