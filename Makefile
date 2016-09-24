
MAKE     = make
CC       = gcc
CFLAGS   = -g -O2 -Wall -Wno-implicit-function-declaration -Wno-unused-variable -D_GNU_SOURCE
LIBS     = -lpthread -lncurses -lbcm2835
PROGNAME = midicube

INCLUDES= -I .

all:
	$(CC) $(CFLAGS) init.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi_in.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi_out.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) mididump.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) misc.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) interface_hw.c -c $(INCLUDES)
	$(CC) $(CFLAGS) interface_hw_lib.c -c $(INCLUDES)
	$(CC) $(CFLAGS) PCD8544.c -c $(INCLUDES)
	$(CC) $(CFLAGS) user_input.c -c $(INCLUDES)
	$(CC) $(CFLAGS) main.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) main.o init.o midi.o midi_in.o midi_out.o mididump.o misc.o interface_hw.o interface_hw_lib.o user_input.o PCD8544.o -o $(PROGNAME) $(LIBS)

clean:
	rm -f *.o *.a $(PROGNAME)
