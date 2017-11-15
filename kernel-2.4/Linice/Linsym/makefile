
INC = /usr/include
H   = ../Include

CC = gcc

CFLAGS = -gstabs+ -Wall -DDBG -O -I$(INC) -I$(H)
#CFLAGS = -Wall -O -I$(INC) -I$(H)


all:	../bin/linsym

clean:
	rm -f ../bin/linsym
	rm -f *.o *~ core

../bin/linsym:	install.o	\
		loader.o	\
		symbols.o   \
        symutils.o  \
        load_elf.o
	$(CC) $^ -o linsym.o
	cp linsym.o ../bin/linsym
	chmod +x ../bin/linsym

install.o:	install.c
	$(CC) $(CFLAGS) -c install.c

loader.o:		loader.c
	$(CC) $(CFLAGS) -c loader.c

symbols.o:		symbols.c
	$(CC) $(CFLAGS) -c symbols.c

symutils.o:		symutils.c
	$(CC) $(CFLAGS) -c symutils.c

load_elf.o:		load_elf.c
	$(CC) $(CFLAGS) -c load_elf.c


