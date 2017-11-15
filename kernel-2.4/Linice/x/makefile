
H    = ../Include
LIBS = -L/usr/X11R6/lib
LIB  = -lX11 -lXxf86dga -lXext

CC = gcc

#CFLAGS = -Wall -O -I$(H)
CFLAGS =  -O -I$(H)

LINKFLAGS = $(LIBS) $(LIB)

all:	../bin/xice

clean:
	rm -f ../bin/xice
	rm -f *.o *~ core

../bin/xice:	xice.o
	$(CC) $^ -o xice $(LINKFLAGS)
	cp xice ../bin/xice
	chmod +x ../bin/xice

xice.o:	xice.c
	$(CC) $(CFLAGS) -c xice.c
