#include "common.h"
#include "global.h"

void main()
 {

  SYS_init();

  SYS_start_task(TASK_MIDI_IN, MIDI_IN_thread, NULL, SCHED_RR, PRIO_VERYHIGH95); 

  if(G_curses_terminal_on)
   SYS_start_task(TASK_KEYBOARD_IN, SYS_keyboard_thread, NULL, SCHED_RR, PRIO_HIGH89);

  SYS_debug(DEBUG_NORMAL,"SYS: librarian startup complete.");  

  while(1)
   {
    if(G_curses_terminal_on)
     SYS_update_status_window();
    usleep(50000);
   }

 }

