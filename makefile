CC = gcc
CFLAGS = -g -Wall
FILES = chip8.c cpu.c
OBJECTS = chip8.o cpu.o
TARGET = chip8

all: $(TARGET)

chip8.o: chip8.c chip8.h

cpu.o: cpu.c chip8.h

chip8: chip8.o cpu.o
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) *.o
