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

#-----------------------------------------------------------------------------
# Set the include path of common Linice include header files
#-----------------------------------------------------------------------------
INCEXT = -I$(PWD)/../include

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

# Compose the final configuration flags

EXTRA_CFLAGS += $(TARGET) $(INCEXT) $(CFLAGS) $(PCIHDR)

obj-m := linice.o

linice-objs := linice_kernel.o_shipped iceface.o

clean:
	rm -f *.o .*.o.cmd .*.ko.cmd *.mod.c
	rm -rf .tmp_versions
