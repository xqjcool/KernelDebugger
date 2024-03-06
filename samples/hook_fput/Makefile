MODNAME = $(shell basename $(src))
ccflags-y := -I$(src)/../../include

obj-m += $(MODNAME).o
$(MODNAME)-objs := module.o ../../src/funchook.o ../../src/procfs.o


