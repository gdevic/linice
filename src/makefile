
INC1 = /usr/include
INC2 = h

CC = gcc
CFLAGS = -Wall -D__KERNEL__ -DMODULE -DLINUX -DDBG -I$(INC1) -I$(INC2)


all: linice.o

clean:
	rm -f *.o *~ core

linice.o: i386.o initial.o display.o command.o history.o debugger.o eval.o dump.o registers.o disassembler.o unassemble.o interrupt.o keyboard.o vga.o convert.o ctype.o malloc.o string.o
	$(LD) -r $^ -o $@


i386.o:	src/i386.asm
	nasm -f elf src/i386.asm -o i386.o

initial.o: src/initial.c
	$(CC) $(CFLAGS) -c src/initial.c

display.o: src/display.c
	$(CC) $(CFLAGS) -c src/display.c

debugger.o: src/debugger.c
	$(CC) $(CFLAGS) -c src/debugger.c

eval.o: src/eval.c
	$(CC) $(CFLAGS) -c src/eval.c

command.o: src/command.c
	$(CC) $(CFLAGS) -c src/command.c

history.o: src/history.c
	$(CC) $(CFLAGS) -c src/history.c

dump.o: src/dump.c
	$(CC) $(CFLAGS) -c src/dump.c

registers.o: src/registers.c
	$(CC) $(CFLAGS) -c src/registers.c

disassembler.o: src/disassembler.c
	$(CC) $(CFLAGS) -c src/disassembler.c

unassemble.o: src/unassemble.c
	$(CC) $(CFLAGS) -c src/unassemble.c

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

string.o: src/string.c
	$(CC) $(CFLAGS) -c src/string.c
