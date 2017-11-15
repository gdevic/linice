
INC = /usr/include
H   = ../include

CC = gcc

CFLAGS = -gstabs+ -Wall -DDBG -O -I$(INC) -I$(H)
#CFLAGS = -Wall -O -I$(INC) -I$(H)


all:	../bin/linsym

clean:
	rm -f ../bin/linsym
	rm -f *.o *~ core

../bin/linsym:	install.o	\
		loader.o	\
		symbols.o
	$(CC) $^ -o linsym.o
	cp linsym.o ../bin/linsym
	chmod +x ../bin/linsym

install.o:	install.c
	$(CC) $(CFLAGS) -c install.c

loader.o:		loader.c
	$(CC) $(CFLAGS) -c loader.c

symbols.o:		symbols.c
	$(CC) $(CFLAGS) -c symbols.c


