
INC1 = /usr/include
INC2 = h

CC = gcc
CFLAGS = -gdwarf-2 -Wall -D__KERNEL__ -DMODULE -DLINUX -DDBG -I$(INC1) -I$(INC2)


all: linice.o

clean:
	rm -f *.o *~ core

linice.o: i386.o iceint.o initial.o interrupt.o keyboard.o vga.o convert.o ctype.o malloc.o printf.o string.o
	$(LD) -r $^ -o $@


i386.o:	src/i386.asm
	nasm -f elf src/i386.asm -o i386.o

iceint.o:	src/iceint.asm
	nasm -f elf src/iceint.asm -o iceint.o

initial.o: src/initial.c
	$(CC) $(CFLAGS) -c src/initial.c

interrupt.o: src/interrupt.c
	$(CC) $(CFLAGS) -c src/interrupt.c

keyboard.o: src/keyboard.c
	$(CC) $(CFLAGS) -c src/keyboard.c

vga.o: src/vga.c
	$(CC) $(CFLAGS) -c src/vga.c

convert.o: src/convert.c
	$(CC) $(CFLAGS) -c src/convert.c

ctype.o: src/ctype.c
	$(CC) $(CFLAGS) -c src/ctype.c

malloc.o: src/malloc.c
	$(CC) $(CFLAGS) -c src/malloc.c

printf.o: src/printf.c
	$(CC) $(CFLAGS) -c src/printf.c

string.o: src/string.c
	$(CC) $(CFLAGS) -c src/string.c
