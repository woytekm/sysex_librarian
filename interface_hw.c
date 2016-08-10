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

void IH_set_keymap_bar(char *key1, char *key2, char *key3, char *key4)
 {
   LCDdrawstring(0,36,key1,TEXT_INVERTED);
   LCDdrawstring(22,36,key2,TEXT_INVERTED);
   LCDdrawstring(44,36,key3,TEXT_INVERTED);
   LCDdrawstring(65,36,key4,TEXT_INVERTED);

   LCDdisplay();
 }


uint8_t IH_sysex_librarian_app(void)
 {

  char *msgbuf;
  uint8_t keymap;

  msgbuf = malloc(15);

  IH_set_keymap_bar("REC","PLY","SAV","CLR");

  LCDdrawstring(0,36,"REC",TEXT_INVERTED);
  LCDdisplay();
 
  while(1)
   {

    read(G_keyboard_event_pipe[0],&keymap,1);

    sprintf(msgbuf,"keymap: %d ",keymap);

    LCDdrawstring(0,9,msgbuf,TEXT_NORMAL);
    LCDdisplay();

   }
 
  return APP_SYSEX_LIBRARIAN;

 }

uint8_t IH_sequencer_app(void)
 {
   return APP_SYSEX_LIBRARIAN;
 }

uint8_t IH_mididump_app(void)
 {
  return APP_SYSEX_LIBRARIAN;
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
