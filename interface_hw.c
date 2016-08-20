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
     IH_set_status_bar(" ABOUT     |  ");
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
  uint8_t do_exit, next_app, edit_result, msg_cnt;

  char *sysex_save_filename; // max filename len is 16 chars
  char *sysex_play_filename;

  char *sysex_msg_info;
 
  scroll_list_item_t *file_list;

  uint32_t total_bytes;

  do_exit = 0;
  sysex_save_filename = malloc(16);
  sysex_msg_info = malloc(16);

  while(!do_exit)
   {

     LCDclear();
     IH_set_keymap_bar("REC","PLY","SAV","CLR");
     IH_set_status_bar(" SYSEX LIB |  ");
     if(G_sysex_record_status == 0)
       LCDdrawstring(0,11,"   rec off", TEXT_NORMAL);
     else
       LCDdrawstring(0,11,"   rec on", TEXT_NORMAL);

     sprintf(sysex_msg_info," rcv:%2d rec:%2d",G_received_sysex_msg_count,G_saved_sysex_msg_count);
     LCDdrawstring(0,21,sysex_msg_info, TEXT_NORMAL);
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
       if(G_sysex_record_status == 0)
         G_sysex_record_status = 1;
       else
         G_sysex_record_status = 0;
       break;

      case KEY2:
       break;

      case KEY3:
       if(G_saved_sysex_msg_count == 0)
        {
         IH_info("no msg to save");
         break;
        }
       memset(sysex_save_filename,32,16);
       sysex_save_filename[15] = 0x0;
       edit_result = IH_edit_string(&sysex_save_filename);
       if(edit_result == 1)
        {
          total_bytes = 0;
          SYS_write_sysex_buffer_to_disk(G_received_sysex_msgs,G_saved_sysex_msg_count,sysex_save_filename);
          for(msg_cnt=1; msg_cnt <= G_saved_sysex_msg_count; msg_cnt++)
           {
            free(G_received_sysex_msgs[msg_cnt].message);
            total_bytes += G_received_sysex_msgs[msg_cnt].length;
            G_received_sysex_msgs[msg_cnt].length = 0;
           }

          SYS_debug(DEBUG_NORMAL,"SYS: wrote sysex buffer to disk (%d messages, %d bytes total)",G_received_sysex_msg_count,total_bytes);
          IH_info("sysex saved");
          G_saved_sysex_msg_count = 0;
        }
       break;

      case KEY4:
       if(G_saved_sysex_msg_count >0)
         {
          for(msg_cnt=1; msg_cnt <= G_saved_sysex_msg_count; msg_cnt++)
           {
            free(G_received_sysex_msgs[msg_cnt].message);
            total_bytes +=G_received_sysex_msgs[msg_cnt].length;
            G_received_sysex_msgs[msg_cnt].length = 0;
           }
          SYS_debug(DEBUG_NORMAL,"SYS: sysex buffer discarded");
          G_saved_sysex_msg_count = 0;
         }
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
