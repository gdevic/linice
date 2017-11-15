##############################################################################
#																			 #
#	Makefile for LINSYM Linice Symbol Loader/Translator						 #
#																			 #
#	(c) 2000-2005 Goran Devic												 #
#	(c) "Linice" by Goran Devic												 #
#	(c) "Linsym" by Goran Devic												 #
#																			 #
##############################################################################

#-----------------------------------------------------------------------------
# List of include directories
#-----------------------------------------------------------------------------

INC = /usr/include
H   = ../include

#-----------------------------------------------------------------------------
# Compiler defines:
#   make 				- engineering non-debug build (default)
#	make debug			- engineering debug build
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Compiler defines: Engineering NON-DEBUG build (default)
#-----------------------------------------------------------------------------

CFLAGS = -Wall -O3 -I$(INC) -I$(H)

#-----------------------------------------------------------------------------
# Compiler defines: Engineering DEBUG build (Development)
#-----------------------------------------------------------------------------
ifeq ($(MAKECMDGOALS),debug)

CFLAGS = -gstabs+ -Wall -DDBG -O -I$(INC) -I$(H)

endif
#-----------------------------------------------------------------------------
# Compiler that is used.
#-----------------------------------------------------------------------------

CC = gcc

#-----------------------------------------------------------------------------
# Formal targets
#-----------------------------------------------------------------------------

debug:	../bin/linsym

nd:		../bin/linsym
	@strip ../bin/linsym

all:	../bin/linsym

clean:
	rm -f ../bin/linsym
	rm -f *.o *~ core

#-----------------------------------------------------------------------------
# List of object modules that build the target
#-----------------------------------------------------------------------------

../bin/linsym:	install.o	\
        ElfToSym.o  \
		ParseDump.o	\
		ParseFunctionLines.o	\
		ParseFunctionScope.o	\
		ParseGlobal.o	\
		ParseStatic.o	\
		ParseSource.o	\
		ParseTypedefs.o	\
		ParseReloc.o	\
		Keymaps.o	\
		Linsym.o	\
		History.o	\
		symbols.o   \
        symutils.o  \
		ChkSym.o
	$(CC) $^ -o ../bin/linsym
	chmod +x ../bin/linsym

install.o:	install.c
	$(CC) $(CFLAGS) -c install.c

ParseDump.o:	ParseDump.c
	$(CC) $(CFLAGS) -c ParseDump.c

ParseFunctionLines.o:	ParseFunctionLines.c
	$(CC) $(CFLAGS) -c ParseFunctionLines.c

ParseFunctionScope.o:	ParseFunctionScope.c
	$(CC) $(CFLAGS) -c ParseFunctionScope.c

ParseGlobal.o:	ParseGlobal.c
	$(CC) $(CFLAGS) -c ParseGlobal.c

ParseStatic.o:	ParseStatic.c
	$(CC) $(CFLAGS) -c ParseStatic.c

ParseSource.o:	ParseSource.c
	$(CC) $(CFLAGS) -c ParseSource.c

ParseTypedefs.o:	ParseTypedefs.c
	$(CC) $(CFLAGS) -c ParseTypedefs.c

ParseReloc.o:	ParseReloc.c
	$(CC) $(CFLAGS) -c ParseReloc.c

Keymaps.o:		Keymaps.c
	$(CC) $(CFLAGS) -c Keymaps.c

Linsym.o:		Linsym.c
	$(CC) $(CFLAGS) -c Linsym.c

History.o:		History.c
	$(CC) $(CFLAGS) -c History.c

symbols.o:		symbols.c
	$(CC) $(CFLAGS) -c symbols.c

ElfToSym.o:		ElfToSym.c
	$(CC) $(CFLAGS) -c ElfToSym.c

symutils.o:		symutils.c
	$(CC) $(CFLAGS) -c symutils.c

ChkSym.o:		ChkSym.c
	$(CC) $(CFLAGS) -c ChkSym.c


