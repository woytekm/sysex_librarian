#include "common.h"
#include "global.h"

int main(int argc, char **argv)
 {

  int c;

  opterr = 0;

  while ((c = getopt(argc, argv, "cbh")) != -1)
   {
     switch (c)
       {
        case 'c':
#ifdef IFACE_CURSES
          G_use_iface_curses = 1;
#else
          printf("curses interface not compiled in.\n");
          exit(-1);
#endif
          break;
        case 'b':
#ifdef IFACE_HW
          G_use_iface_hw = 1;
#else
          printf("hardware interface not compiled in.\n");
          exit(-1);
#endif
          break;
        case 'h':
          SYS_showhelp();
          exit(-1);
          break;
        case '?':
          if (isprint (optopt))
           {
            fprintf (stderr, "ERROR: Unknown option `-%c'.\n", optopt);
            SYS_showhelp();
           }
          else
           {
            fprintf (stderr,"ERROR: Unknown option character `\\x%x'.\n",
                     optopt);
            SYS_showhelp();
           }
        }
    }

   if(optind < argc) 
    { 
     fprintf(stderr,"ERROR: invalid non-option arguments.\n"); 
     SYS_showhelp(); 
    }

  SYS_init();

  SYS_start_task(TASK_MIDI_IN, MIDI_IN_thread, NULL, SCHED_RR, PRIO_VERYHIGH95); 

#ifdef IFACE_CURSES
  if(G_use_iface_curses == 1)
   SYS_start_task(TASK_KEYBOARD_IN, SYS_keyboard_thread, NULL, SCHED_RR, PRIO_HIGH89);
#endif

#ifdef IFACE_HW
  if(G_use_iface_hw == 1)
   {
    SYS_start_task(TASK_KEYBOARD_IN, SYS_shiftin_thread, NULL, SCHED_RR, PRIO_VERYHIGH90);
    SYS_start_task(TASK_INTERFACE_HW, SYS_hw_interface_thread, NULL, SCHED_RR, PRIO_VERYHIGH91);   
    SYS_start_task(TASK_MIDI_INOUT_INDICATOR, IH_MIDI_inout_indicator, NULL, SCHED_RR, PRIO_VERYHIGH92);
   }
#endif

  SYS_debug(DEBUG_NORMAL,"SYS: MidiCube startup complete.");  

  while(1)
   {
#ifdef IFACE_CURSES
    if(G_use_iface_curses == 1)
     SYS_update_status_window();
#endif
    usleep(50000);
   }

 }

