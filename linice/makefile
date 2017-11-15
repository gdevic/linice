INC = /usr/include/linux
H1  = ../include
H2  = include

DEF = -D__KERNEL__ -D_LOOSE_KERNEL_NAMES -DMODULE -DLINUX -DDBG

CC = gcc
AS = nasm

CFLAGS = -gstabs+ -Wall $(DEF) -O -I$(INC) -I$(H1) -I$(H2)
#CFLAGS =         -Wall $(DEF) -O -I$(INC) -I$(H1) -I$(H2)


all:	../bin/linice.o

clean:
	rm -f ../bin/*
	rm -f *.o *~ core

../bin/linice.o:	driver.o	\
			malloc.o	\
			eval.o		\
			init.o		\
			edlin.o		\
			debugger.o	\
			i386.o		\
			interrupt.o	\
			command.o	\
			registers.o	\
			data.o		\
			code.o		\
			history		\
			input.o		\
			keyboard.o	\
			serial_in.o	\
			output.o	\
			window.o	\
			vga.o
	$(LD) -r $^ -o linice.o
	cp linice.o ../bin/linice.o
	cp linice.dat ../bin/linice.dat

driver.o:	driver.c
	$(CC) $(CFLAGS) -c driver.c

malloc.o:	malloc.c
	$(CC) $(CFLAGS) -c malloc.c

init.o:		init.c
	$(CC) $(CFLAGS) -c init.c

i386.o:		i386.asm
	$(AS) -f elf i386.asm -o i386.o

interrupt.o:		interrupt.c
	$(CC) $(CFLAGS) -c interrupt.c

debugger.o:	command/debugger.c
	$(CC) $(CFLAGS) -c command/debugger.c

edlin.o:	command/edlin.c
	$(CC) $(CFLAGS) -c command/edlin.c

command.o:	command/command.c
	$(CC) $(CFLAGS) -c command/command.c

eval.o:		command/eval.c
	$(CC) $(CFLAGS) -c command/eval.c

registers.o:		command/registers.c
	$(CC) $(CFLAGS) -c command/registers.c

data.o:		command/data.c
	$(CC) $(CFLAGS) -c command/data.c

code.o:		command/code.c
	$(CC) $(CFLAGS) -c command/code.c

history.o:	history.c
	$(CC) $(CFLAGS) -c history.c

input.o:	input/input.c
	$(CC) $(CFLAGS) -c input/input.c

keyboard.o:	input/keyboard.c
	$(CC) $(CFLAGS) -c input/keyboard.c

serial_in.o:	input/serial.c
	$(CC) $(CFLAGS) -c input/serial.c -o serial_in.o

mouse.o:	input/mouse.c
	$(CC) $(CFLAGS) -c input/mouse.c

output.o:	output/output.c
	$(CC) $(CFLAGS) -c output/output.c

window.o:	output/window.c
	$(CC) $(CFLAGS) -c output/window.c

vga.o:		output/vga.c
	$(CC) $(CFLAGS) -c output/vga.c


