obj-m += my_mod_rand.o
KDIR = /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=`pwd` modules
clean:
	make -C $(KDIR) M=`pwd` clean
test: 
	gcc test.c -o test.o -Wall
