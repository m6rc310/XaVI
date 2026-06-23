#DEBUG = -g -DDEBUG -nostdinc -nostdlib
#DEBUG = -UDEBUG -nostdinc -nostdlib
DEBUG = -UDEBUG

XaVI:		XaVI.o tools.o syscalltool.o screen.o
	gcc $(DEBUG) -o XaVI XaVI.o tools.o syscalltool.o screen.o -lX11

XaVI.o:		XaVI.c define.h struct.h
	gcc $(DEBUG) -c XaVI.c

screen.o:       screen.c
	gcc $(DEBUG) -c screen.c

tools.o:		tools.c struct.h
	gcc $(DEBUG) -c tools.c

syscalltool.o:		syscalltool.asm
	nasm -f elf64 syscalltool.asm

clean:
		rm *.o XaVI
