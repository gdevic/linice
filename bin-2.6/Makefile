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


obj-m := linice.o

linice-objs := linice_kernel.o iceface.o
