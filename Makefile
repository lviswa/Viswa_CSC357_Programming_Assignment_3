OBJS    = fs_emulator.o
SOURCE  = fs_emulator.c
OUT = fs_emulator
CC   = gcc
FLAGS    = -g -c -Wall -pedantic -std=c99
LFLAGS   = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

fs_emulator.o: fs_emulator.c
	$(CC) $(FLAGS) fs_emulator.c 


clean:
	rm -f $(OBJS) $(OUT)
