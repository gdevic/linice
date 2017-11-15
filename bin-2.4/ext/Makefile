#=============================================================================
#
# Makefile for the sample debugger extension module
#
#=============================================================================

#-----------------------------------------------------------------------------
# Set the include path of the kernel headers
#-----------------------------------------------------------------------------
INC = /lib/modules/`uname -r`/build/include

#-----------------------------------------------------------------------------
# Set the include path of the Linice extension header file
#-----------------------------------------------------------------------------
INCEXT = ../../include

#-----------------------------------------------------------------------------
# Set the compiler flags
#-----------------------------------------------------------------------------
CFLAGS = -gstabs+ -Wall -O -DDBG
#CFLAGS = -Wall -O

DEF = $(TARGET) -I$(INC) -I$(INCEXT) -D__KERNEL__ -D_LOOSE_KERNEL_NAMES -DMODULE -DLINUX

CC = gcc


extension.o:	extension.c
	$(CC) $(CFLAGS) $(DEF) -c extension.c -o extension.o

clean:
	rm -f extension.o
