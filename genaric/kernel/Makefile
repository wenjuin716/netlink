KDIR := /lib/modules/$(shell uname -r)/build

#obj-m += netlinkKernel.o
obj-m += netlinkKernel.o rmmod_force.o
#GENL_CFLAGS= -DNETLINK_UNICAST -DNETLINK_MULTICAST
GENL_CFLAGS= -DNETLINK_UNICAST
#GENL_CFLAGS= -DNETLINK_MULTICAST
CFLAGS_netlinkKernel.o := $(GENL_CFLAGS)
all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
