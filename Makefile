KDIR		?= /lib/modules/$(shell uname -r)/build
ARCH		?=  $(shell uname -m)
TARGET		?= sample


EXTRA_CFLAGS    =  -Wall -o2
SRC_FILES       =   funchook.c module.c

ifdef CONFIG_PROC_FS
	SRC_FILES += procfs.c
endif

OBJ_FILES       =   $(addsuffix .o, $(basename $(SRC_FILES)))

$(TARGET)-y 	= $(OBJ_FILES)
obj-m 		+= $(TARGET).o

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
