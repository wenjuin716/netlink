KDIR := /lib/modules/$(shell uname -r)/build

obj-m += netlinkKernel.o
CFLAGS_netlinkKernel.o := -DNETLINK_UNICAST -DNETLINK_MULTICAST
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
