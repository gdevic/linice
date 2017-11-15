#=============================================================================
#
# Final user Makefile for the linice module
#
#=============================================================================

# Set the configure defines that the target kernel was compiled with

#TARGET = -DSMP -DIO_APIC
TARGET =

# Set the include path of kernel headers that this module will be supporting
INC = /usr/include

# Set the compiler flags - note that you have to specify at least -O when
# compiling a Linux kernel
#CFLAGS = -gstabs+ -Wall -O
CFLAGS = -Wall -O
#CFLAGS = -O

#DEF = $(TARGET) -I$(INC) -D__KERNEL__ -D_LOOSE_KERNEL_NAMES -DMODULE -DLINUX -DDBG
DEF = $(TARGET) -I$(INC) -D__KERNEL__ -D_LOOSE_KERNEL_NAMES -DMODULE -DLINUX

CC = gcc


linice.o:	linice_kernel.o iceface.o
	$(LD) -r $^ -o ../bin/linice.o
	@echo ------------------------------------------------------------------------------
	@echo Creating linice_`uname -r` and copying kernel-dependent module...
	@mkdir -p linice_`uname -r`
	@cp -f linice.o linice_`uname -r`
	@rm -f linice.o
	@echo ------------------------------------------------------------------------------
	@echo Done.

iceface.o:	iceface.c
	$(CC) $(CFLAGS) $(DEF) -c iceface.c -o iceface.o


clean:
	rm -f iceface.o
	rm -f linice_`uname -r`/linice.o
	rmdir linice_`uname -r`
