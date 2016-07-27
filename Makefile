
MAKE     = make
CC       = gcc
CFLAGS   = -g -O3 -Wall -Wno-implicit-function-declaration -Wno-unused-variable
LIBS     = -lpthread -lncurses
PROGNAME = sysex_librarian

INCLUDES= -I .

all:
	$(CC) $(CFLAGS) init.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi_in.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) midi_out.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) misc.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) interface_curses.c -c $(INCLUDES)
	$(CC) $(CFLAGS) user_input.c -c $(INCLUDES)
	$(CC) $(CFLAGS) main.c -c  $(INCLUDES)
	$(CC) $(CFLAGS) main.o init.o midi.o midi_in.o midi_out.o misc.o interface_curses.o user_input.o -o $(PROGNAME) $(LIBS)

clean:
	rm -f *.o *.a $(PROGNAME)
