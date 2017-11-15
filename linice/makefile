# INC = /usr/include/linux
INC = /usr/src/linux/include/
H1  = ../Include
H2  = include

DEF = -D__KERNEL__ -D_LOOSE_KERNEL_NAMES -DMODULE -DLINUX -DDBG

CC = gcc
AS = nasm

#CFLAGS = -gstabs+ -Wall $(DEF) -O -I$(INC) -I$(H1) -I$(H2)
CFLAGS =         -Wall $(DEF) -O -I$(INC) -I$(H1) -I$(H2)


all:	../bin/linice.o

clean:
	rm -f ../bin/linice.o
	rm -f *.o *~ core

../bin/linice.o:	driver.o	\
			malloc.o	\
			memaccess.o	\
			eval.o		\
			init.o		\
			symbolTable.o	\
			symbols.o	\
			proc.o		\
			edlin.o		\
			debugger.o	\
			disassembler.o	\
			disassembler-bytelen.o	\
			i386.o		\
			interrupt.o	\
			command.o	\
			registers.o	\
			data.o		\
			code.o		\
			customization.o	\
			windowcontrol.o	\
			blockops.o		\
			ioport.o	\
			sysinfo.o	\
			page.o		\
			breakpoints.o	\
			pci.o		\
			flow.o		\
			history.o	\
			input.o		\
			keyboard.o	\
			vt100i.o	\
			mouse.o		\
			serial.o	\
			output.o	\
			window.o	\
			vga.o		\
			lfb.o		\
            vt100o.o
	$(LD) -r $^ -o ../bin/linice.o

driver.o:	driver.c
	$(CC) $(CFLAGS) -c driver.c

malloc.o:	malloc.c
	$(CC) $(CFLAGS) -c malloc.c

memaccess.o:	memaccess.c
	$(CC) $(CFLAGS) -c memaccess.c

init.o:		init.c
	$(CC) $(CFLAGS) -c init.c

symbols.o:		symbolTable.c
	$(CC) $(CFLAGS) -c symbolTable.c

symbols.o:		symbols.c
	$(CC) $(CFLAGS) -c symbols.c

proc.o:		proc.c
	$(CC) $(CFLAGS) -c proc.c

i386.o:		i386.asm
	$(AS) -f elf i386.asm -o i386.o

interrupt.o:		interrupt.c
	$(CC) $(CFLAGS) -c interrupt.c

history.o:	history.c
	$(CC) $(CFLAGS) -c history.c

debugger.o:	command/debugger.c
	$(CC) $(CFLAGS) -c command/debugger.c

disassembler.o:	command/disassembler.c
	$(CC) $(CFLAGS) -c command/disassembler.c

disassembler-bytelen.o:	command/disassembler-bytelen.c
	$(CC) $(CFLAGS) -c command/disassembler-bytelen.c

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

customization.o:		command/customization.c
	$(CC) $(CFLAGS) -c command/customization.c

windowcontrol.o:		command/windowcontrol.c
	$(CC) $(CFLAGS) -c command/windowcontrol.c

blockops.o:		command/blockops.c
	$(CC) $(CFLAGS) -c command/blockops.c

ioport.o:		command/ioport.c
	$(CC) $(CFLAGS) -c command/ioport.c

sysinfo.o:		command/sysinfo.c
	$(CC) $(CFLAGS) -c command/sysinfo.c

page.o:		    command/page.c
	$(CC) $(CFLAGS) -c command/page.c

breakpoints.o:	command/breakpoints.c
	$(CC) $(CFLAGS) -c command/breakpoints.c

pci.o:		    command/pci.c
	$(CC) $(CFLAGS) -c command/pci.c

flow.o:		command/flow.c
	$(CC) $(CFLAGS) -c command/flow.c

input.o:	input/input.c
	$(CC) $(CFLAGS) -c input/input.c

keyboard.o:	input/keyboard.c
	$(CC) $(CFLAGS) -c input/keyboard.c

vt100i.o:	input/vt100.c
	$(CC) $(CFLAGS) -c input/vt100.c -o vt100i.o

mouse.o:	input/mouse.c
	$(CC) $(CFLAGS) -c input/mouse.c

serial.o:	serial.c
	$(CC) $(CFLAGS) -c serial.c -o serial.o

output.o:	output/output.c
	$(CC) $(CFLAGS) -c output/output.c

window.o:	output/window.c
	$(CC) $(CFLAGS) -c output/window.c

vga.o:		output/vga.c
	$(CC) $(CFLAGS) -c output/vga.c

lfb.o:		output/lfb.c
	$(CC) $(CFLAGS) -c output/lfb.c

vt100o.o:	output/vt100.c
	$(CC) $(CFLAGS) -c output/vt100.c -o vt100o.o


