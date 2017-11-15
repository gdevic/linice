
INC = /usr/include
H   = ../include

CC = gcc

CFLAGS = -gstabs+ -Wall -D__KERNEL__ -DMODULE -DLINUX -DDBG -O -I$(INC) -I$(H)
#CFLAGS = -Wall -D__KERNEL__ -DMODULE -DLINUX -O -I$(INC) -I$(H)


all:	../bin/linice.o

clean:
	rm -f ../bin/*
	rm -f *.o *~ core

../bin/linice.o:	driver.o	\
			malloc.o	\
			eval.o		\
			clib.o
	$(LD) -r $^ -o linice.o
	cp linice.o ../bin/linice.o

driver.o:	driver.c
	$(CC) $(CFLAGS) -c driver.c

malloc.o:	malloc.c
	$(CC) $(CFLAGS) -c malloc.c

eval.o:		eval.c
	$(CC) $(CFLAGS) -c eval.c

clib.o:		clib.c
	$(CC) $(CFLAGS) -c clib.c


