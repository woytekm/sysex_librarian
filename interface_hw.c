#include "common.h"
#include "global.h"

void IH_startup_display(void)
 {
    char *msgbuf;
   
    msgbuf = malloc(16);

    sprintf(msgbuf," version %d.%d",VER_MAJ,VER_MIN);
    LCDdrawstring(0,1,"  MIDI cube  ", TEXT_NORMAL);
    LCDdrawstring(0,9,msgbuf,TEXT_NORMAL);
    LCDdisplay();

    free(msgbuf);

 }

uint8_t IH_sysex_librarian_app(void)
 {

   

 }

uint8_t IH_sequencer_app(void)
 {


 }

uint8_t IH_mididump_app(void)
 {


 }

void SYS_hw_interface_thread(void *params)
 {

   uint8_t next_app;

   IH_startup_display();
   sleep(5);
   LCDclear();

   // default app
   next_app = IH_sysex_librarian_app();

   while(1)
    {

      switch(next_app)
       {

        case APP_SYSEX_LIBRARIAN:
         next_app = IH_sysex_librarian_app();

        case APP_SEQUENCER:
         next_app = IH_sequencer_app();

        case APP_MIDIDUMP:
         next_app = IH_mididump_app();

       }
     
      sleep(1);

    }

 }
