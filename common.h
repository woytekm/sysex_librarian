#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <poll.h>
#include <strings.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <dirent.h>
#include <stropts.h>

/* serial port includes  */
#include <termio.h>
#include <fcntl.h>
#include <err.h>
#include <linux/serial.h>

/* UDP receiver includes  */
#include <sys/socket.h>
#include <netinet/in.h>

/* scheduling  */
#include <linux/sched.h>

#ifdef IFACE_CURSES
/* curses */
#include <ncurses.h>
#endif

#ifdef IFACE_HW

#include "bcm2835.h"

#endif

