obj-m := mypipe.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

insert:
	sudo insmod mypipe.ko
	dmesg | tail -n 1

install: insert
	sudo mknod /dev/mypipe c $(shell cat /proc/devices | grep MyPipe | awk '{print $$1}') 0

uninstall:
	sudo rmmod mypipe
	dmesg | tail -n 1
	sudo rm -f /dev/mypipe

test:
	gcc readtest.c -o readtest.exe
	gcc writetest.c -o writetest.exe

.PHONY: clean
clean distclean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -f readtest.exe writetest.exe