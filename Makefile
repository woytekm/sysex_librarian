
MAKE     = make
CC       = gcc
CFLAGS   = -g -O2 -Wall -Wno-implicit-function-declaration -Wno-unused-variable -DIFACE_CURSES -DIFACE_HW
LIBS     = -lpthread -lncurses -lbcm2835
PROGNAME = sysex_librarian

INCLUDES= -I .

all:
	$(CC) $(CFLAGS) init.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi_in.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi_out.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) misc.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) interface_hw.c -c $(INCLUDES)
	$(CC) $(CFLAGS) interface_curses.c -c $(INCLUDES)
	$(CC) $(CFLAGS) PCD8544.c -c $(INCLUDES)
	$(CC) $(CFLAGS) user_input.c -c $(INCLUDES)
	$(CC) $(CFLAGS) main.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) main.o init.o midi.o midi_in.o midi_out.o misc.o interface_hw.o interface_curses.o user_input.o PCD8544.o -o $(PROGNAME) $(LIBS)

clean:
	rm -f *.o *.a $(PROGNAME)
