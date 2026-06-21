obj-m := procusers.o

KDIR := /home/aleksa/linux-6.12

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean