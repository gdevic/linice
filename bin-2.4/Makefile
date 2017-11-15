#=============================================================================
#
# Final user Makefile for the Linice module
#
#=============================================================================

#=============================================================================
# Start of the user configuration section
#=============================================================================
# Set the configure defines that the target kernel was compiled with. This has
# to match the target kernel.

#-----------------------------------------------------------------------------
# Select the optional IO APIC and SMP machine build
#-----------------------------------------------------------------------------
#TARGET = -DSMP -DIO_APIC
TARGET =

#-----------------------------------------------------------------------------
# Set the include path of kernel headers that this module will be supporting
#-----------------------------------------------------------------------------
INC = /lib/modules/`uname -r`/build/include

#-----------------------------------------------------------------------------
# Set the include path of common Linice include header files
#-----------------------------------------------------------------------------
INCEXT = ../include

#-----------------------------------------------------------------------------
# Set the compiler flags
#-----------------------------------------------------------------------------
CFLAGS = -gstabs+ -Wall -O -DDBG
#CFLAGS = -Wall -O

#-----------------------------------------------------------------------------
# Enable if you wish to include full PCI strings header, or comment it out
# to make the final module a bit smaller (by about ~300K)
#-----------------------------------------------------------------------------
#PCIHDR = PCIHDR

#=============================================================================
# End of the user configuration section
#=============================================================================

DEF = $(TARGET) -I$(INC) -I$(INCEXT) -D__KERNEL__ -D_LOOSE_KERNEL_NAMES -DMODULE -DLINUX -D_$(PCIHDR)

CC = gcc


linice.o:	linice_kernel.o iceface.o
	$(LD) -r $^ -o linice.o
	@echo ------------------------------------------------------------------------------
	@echo Creating linice_`uname -r` and copying kernel-dependent module...
	@mkdir -p linice_`uname -r`
	@cp -f linice.o linice_`uname -r`/linice.o
	@rm -f linice.o
	@echo ------------------------------------------------------------------------------
	@echo Done. You may now run \"linsym -i\" to install the linice module.

iceface.o:	iceface.c
	$(CC) $(CFLAGS) $(DEF) -c iceface.c -o iceface.o

clean:
	rm -f iceface.o
	rm -f linice_`uname -r`/linice.o
	rmdir linice_`uname -r`
