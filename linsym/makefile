INC = /usr/include
H   = ../include

CC = gcc

#CFLAGS = -O -I$(INC) -I$(H)
CFLAGS = -gstabs+ -Wall -DDBG -O -I$(INC) -I$(H)
#CFLAGS = -Wall -O -I$(INC) -I$(H)


all:	../bin/linsym

clean:
	rm -f ../bin/linsym
	rm -f *.o *~ core

../bin/linsym:	install.o	\
        load_elf.o  \
		ParseFunctionLines.o	\
		ParseFunctionScope.o	\
		ParseGlobal.o	\
		ParseSource.o	\
		ParseTypedefs.o	\
		ParseReloc.o	\
		HashTable.o	\
		Keymaps.o	\
		loader.o	\
		History.o	\
		symbols.o   \
        symutils.o
	$(CC) $^ -o linsym.o
	cp linsym.o ../bin/linsym
	chmod +x ../bin/linsym

install.o:	install.c
	$(CC) $(CFLAGS) -c install.c

ParseFunctionLines.o:	ParseFunctionLines.c
	$(CC) $(CFLAGS) -c ParseFunctionLines.c

ParseFunctionScope.o:	ParseFunctionScope.c
	$(CC) $(CFLAGS) -c ParseFunctionScope.c

ParseGlobal.o:	ParseGlobal.c
	$(CC) $(CFLAGS) -c ParseGlobal.c

ParseSource.o:	ParseSource.c
	$(CC) $(CFLAGS) -c ParseSource.c

ParseTypedefs.o:	ParseTypedefs.c
	$(CC) $(CFLAGS) -c ParseTypedefs.c

ParseReloc.o:	ParseReloc.c
	$(CC) $(CFLAGS) -c ParseReloc.c

HashTable.o:		HashTable.c
	$(CC) $(CFLAGS) -c HashTable.c

Keymaps.o:		Keymaps.c
	$(CC) $(CFLAGS) -c Keymaps.c

loader.o:		loader.c
	$(CC) $(CFLAGS) -c loader.c

History.o:		History.c
	$(CC) $(CFLAGS) -c History.c

symbols.o:		symbols.c
	$(CC) $(CFLAGS) -c symbols.c

load_elf.o:		load_elf.c
	$(CC) $(CFLAGS) -c load_elf.c

symutils.o:		symutils.c
	$(CC) $(CFLAGS) -c symutils.c


