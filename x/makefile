##############################################################################
#																			 #
#	Makefile for XICE - X-Window Linice Loader								 #
#																			 #
#	(c) 2000-2004 Goran Devic												 #
#	(c) "Linice" by Goran Devic												 #
#	(c) "Linsym" by Goran Devic												 #
#																			 #
##############################################################################

#-----------------------------------------------------------------------------
# List of include directories
#-----------------------------------------------------------------------------

H    = ../include
LIBS = -L/usr/X11R6/lib
LIB  = -lX11 -lXxf86dga -lXext

LINKFLAGS = $(LIBS) $(LIB)

#-----------------------------------------------------------------------------
# Compiler defines:
#	make retail			- retail build
#	make debug			- engineering debug build (default)
#   make nd				- engineering non-debug build
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Compiler defines: Engineering DEBUG build (Development, default)
#-----------------------------------------------------------------------------

CFLAGS = -Wall -O -I$(H)

#-----------------------------------------------------------------------------
# Compiler defines: Engineering NON-DEBUG build
#-----------------------------------------------------------------------------
ifeq ($(MAKECMDGOALS),nd)

CFLAGS = -O3 -I$(H)

endif
#-----------------------------------------------------------------------------
# Compiler defines: RETAIL build
#-----------------------------------------------------------------------------
ifeq ($(MAKECMDGOALS),retail)

CFLAGS = -O3 -I$(H)

endif
#-----------------------------------------------------------------------------
# Compiler that is used.
#-----------------------------------------------------------------------------

CC = gcc

#-----------------------------------------------------------------------------
# Formal targets
#-----------------------------------------------------------------------------

debug:	../bin/xice

nd:		../bin/xice

retail:	../bin/xice
	@strip ../bin/xice

all:	../bin/xice

clean:
	rm -f ../bin/xice
	rm -f *.o *~ core

#-----------------------------------------------------------------------------
# List of object modules that build the target
#-----------------------------------------------------------------------------

../bin/xice:	xice.o
	$(CC) $^ -o xice $(LINKFLAGS)
	cp xice ../bin/xice
	chmod +x ../bin/xice

xice.o:	xice.c
	$(CC) $(CFLAGS) -c xice.c
