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

  SYS_start_task(TASK_KEYBOARD_IN, SYS_shiftin_thread, NULL, SCHED_RR, PRIO_VERYHIGH90);
  SYS_start_task(TASK_INTERFACE_HW, SYS_hw_interface_thread, NULL, SCHED_RR, PRIO_VERYHIGH91);   
  SYS_start_task(TASK_MIDI_INOUT_INDICATOR, IH_MIDI_inout_indicator, NULL, SCHED_RR, PRIO_VERYHIGH92);
  SYS_start_task(TASK_MIDI_IN_TIMER, SYS_MIDI_IN_timer, NULL, SCHED_RR, PRIO_VERYHIGH99);

  SYS_debug(DEBUG_NORMAL,"SYS: MidiCube startup complete.");  

  while(1)
   {
    sleep(1);
   }

 }

