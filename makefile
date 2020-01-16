CC = gcc
CFLAGS = -g -Wall
FILES = chip8.c cpu.c
OBJECTS = chip8.o cpu.o

chip8: chip8.o cpu.o
	$(CC) $(CFLAGS) -o chip8 $(OBJECTS)

chip8.o:
	$(CC) $(CFLAGS) -c chip8.c

cpu.o:
	$(CC) $(CFLAGS) -c cpu.c

clean:
	rm -f chip8 *.o
