#include "common.h"
#include "global.h"

uint8_t IH_choose_app()
 {

  scroll_list_item_t *first_item;
  scroll_list_item_t *next_item;
  uint8_t list_return_code;
  
  first_item = IH_scroll_list_item_add(NULL,"SYSEX LIB     ",APP_SYSEX_LIBRARIAN);
  next_item = IH_scroll_list_item_add(first_item,"MIDI SEQ      ",APP_SEQUENCER);
  next_item = IH_scroll_list_item_add(next_item,"MIDI DUMP     ",APP_MIDIDUMP); 
  next_item = IH_scroll_list_item_add(next_item,"ABOUT         ",APP_ABOUT); 

  list_return_code = IH_scroll_list(first_item,"Choose app|   ");
  IH_scroll_list_destroy(first_item);
  return list_return_code;
  
 }

uint8_t IH_about_app(void)
 {
  uint8_t key_event;
  uint8_t do_exit, next_app;
  do_exit = 0;

  while(!do_exit)
   {
     LCDclear();
     IH_startup_display(1);
     IH_set_keymap_bar("","","","");
     IH_set_status_bar(" ABOUT   |  ");
     LCDdisplay();

    SYS_debug(DEBUG_NORMAL,"reading key events");

    read(G_keyboard_event_pipe[0],&key_event,1);

    SYS_debug(DEBUG_NORMAL,"key event: %d",key_event);

    switch(key_event)
     {
      case ENC_KEY:
       next_app = IH_choose_app();
       if(next_app != 0)
         return next_app;
       break;

      case KEY1:
       break;

      case KEY2:
       break;

      case KEY3:
       break;

      case KEY4:
       break;

     }

    LCDclear();
    LCDdisplay();
   }

  return APP_ABOUT;

 }

uint8_t IH_sysex_librarian_app(void)
 {

  uint8_t key_event;
  uint8_t do_exit, next_app;

  do_exit = 0;

  while(!do_exit)
   {

     LCDclear();
     IH_set_keymap_bar("REC","PLY","SAV","CLR");
     IH_set_status_bar(" SYSEX APP |  ");
     LCDdisplay();

    SYS_debug(DEBUG_NORMAL,"reading key events");

    read(G_keyboard_event_pipe[0],&key_event,1);

    SYS_debug(DEBUG_NORMAL,"key event: %d",key_event);

    switch(key_event)
     {
      case ENC_KEY:
       next_app = IH_choose_app();
       if(next_app != 0)
         return next_app; 
       break;

      case KEY1:
       break;

      case KEY2:
       break;

      case KEY3:
       break;

      case KEY4:
       break;

     } 
 
    LCDclear();
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

   IH_startup_display(0);
   sleep(1);
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

        case APP_ABOUT:
         next_app = IH_about_app();

       }
     
      sleep(1);

    }

 }
