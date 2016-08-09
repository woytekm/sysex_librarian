#include "common.h"

#define MIDI_IN_BUFLEN 1024000

#define PRIO_VERYHIGH99 99
#define PRIO_VERYHIGH98 98
#define PRIO_VERYHIGH97 97
#define PRIO_VERYHIGH96 96
#define PRIO_VERYHIGH95 95
#define PRIO_VERYHIGH94 94
#define PRIO_VERYHIGH93 93
#define PRIO_VERYHIGH92 92
#define PRIO_VERYHIGH91 91
#define PRIO_VERYHIGH90 90

#define PRIO_HIGH89 89
#define PRIO_HIGH88 88
#define PRIO_HIGH87 87
#define PRIO_HIGH86 86
#define PRIO_HIGH85 85
#define PRIO_HIGH84 84
#define PRIO_HIGH83 83
#define PRIO_HIGH82 82
#define PRIO_HIGH81 81
#define PRIO_HIGH80 80

#define PRIO_NORMAL59 59
#define PRIO_NORMAL58 58
#define PRIO_NORMAL57 57
#define PRIO_NORMAL56 56
#define PRIO_NORMAL55 55
#define PRIO_NORMAL54 54
#define PRIO_NORMAL53 53
#define PRIO_NORMAL52 52
#define PRIO_NORMAL51 51
#define PRIO_NORMAL50 50

#define PRIO_LOW29 29
#define PRIO_LOW28 28
#define PRIO_LOW27 27
#define PRIO_LOW26 26
#define PRIO_LOW25 25
#define PRIO_LOW24 24
#define PRIO_LOW23 23
#define PRIO_LOW22 22
#define PRIO_LOW21 21
#define PRIO_LOW20 20

#define TASK_MIDI_IN 1
#define TASK_MIDI_OUT 2
#define TASK_KEYBOARD_IN 3
#define TASK_GPIO_IN 4
#define TASK_GPIO_LED 5
#define TASK_INTERFACE_HW 6

#define DEBUG_NORMAL 2
#define DEBUG_HIGH 3
#define DEBUG_VERYHIGH 4

#define MAX_TASK 8

#define MAXMSG 255

#define DEBUG_LOW 1
#define DEBUG_NORMAL 2
#define DEBUG_HIGH 3
#define DEBUG_VERYHIGH 4

#define DEBUG 1
#define DEBUG_LEVEL 2

#define DEFAULT_THREAD_STACK_SIZE 2048000

int     G_MIDI_fd;
uint8_t G_MIDI_msg_lengths[256];

typedef struct _task_t {
  pthread_t task_id;
  int input_pipe[2];
 } task_t;

task_t G_tasks[MAX_TASK];
uint8_t G_task_count;

#define MAX_SYSEX_MSGS 1024

typedef struct _sysex_msg_t {
  uint32_t length;
  unsigned char *message;
 } sysex_msg_t;

sysex_msg_t G_received_sysex_msgs[MAX_SYSEX_MSGS];
uint16_t G_received_sysex_msg_count;
uint16_t G_saved_sysex_msg_count;

unsigned char *G_sysex_transmit_buffer;

void MIDI_IN_thread(void *params);
void SYS_keyboard_thread(void *params);
void SYS_shiftin_thread(void *params);
void SYS_hw_interface_thread(void *params);

uint8_t G_curses_terminal_on;

#define PANEL_WIDTH 95
#define PANEL_HEIGHT 30

WINDOW *G_menu_bar, *G_log_window, *G_border_log_window, *G_status_window, *G_border_status_window;

uint8_t G_log_to_curses;
 
uint8_t G_sysex_record_status;
uint8_t G_write_to_file;

uint8_t G_use_iface_curses;
uint8_t G_use_iface_hw;
uint8_t G_global_keymap;

#define SHIFTIN_DELAY 4000

#define GPIO17_PL_PIN11 RPI_V2_GPIO_P1_11
#define GPIO27_CP_PIN13 RPI_V2_GPIO_P1_13
#define GPIO22_CE_PIN15 RPI_V2_GPIO_P1_15
#define GPIO23_Q7_PIN16 RPI_V2_GPIO_P1_16

#define DIN1  RPI_V2_GPIO_P1_35
#define SCLK1 RPI_V2_GPIO_P1_37
#define DC1   RPI_V2_GPIO_P1_33
#define RST1  RPI_V2_GPIO_P1_29
#define CS1   RPI_V2_GPIO_P1_31


#define VER_MAJ 0
#define VER_MIN 1

#define TEXT_INVERTED 1
#define TEXT_NORMAL   0

#define APP_SYSEX_LIBRARIAN 10
#define APP_SEQUENCER       20
#define APP_MIDIDUMP        30


// end of global.h

