#DEBUG = -g -DDEBUG -nostdinc -nostdlib
DEBUG = -UDEBUG -nostdinc -nostdlib

asm:	asm.o tools.o syscalltool.o
	gcc $(DEBUG) -o asm asm.o tools.o syscalltool.o

asm.o:		asm.c define.h struct.h
	gcc $(DEBUG) -c asm.c

tools.o:		tools.c struct.h
	gcc $(DEBUG) -c tools.c

syscalltool.o:		syscalltool.asm
	nasm -f elf64 syscalltool.asm

clean:
		rm *.o asm
