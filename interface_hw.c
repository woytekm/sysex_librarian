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

  list_return_code = IH_scroll_list(first_item,"Choose app    ");
  SYS_debug(DEBUG_NORMAL,"Scroll list returned: %d",list_return_code);
  IH_scroll_list_destroy(first_item);
  return list_return_code;
  
 }

uint8_t IH_about_app(void)
 {
  uint8_t key_event;
  uint8_t do_exit, next_app;
  do_exit = 0;

  G_active_app = APP_ABOUT;

  while(!do_exit)
   {
     LCDclear();
     IH_startup_display(1);
     IH_set_keymap_bar("","","","");
     IH_set_status_bar(" ABOUT        ");
     pthread_mutex_lock(&G_display_lock);
     LCDdisplay();
     pthread_mutex_unlock(&G_display_lock);

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
    pthread_mutex_lock(&G_display_lock);
    LCDdisplay();
    pthread_mutex_unlock(&G_display_lock);
   }

  return APP_ABOUT;

 }


uint8_t IH_sysex_librarian_app(void)
 {

  uint8_t key_event,lcd_needs_update;
  uint8_t do_exit, next_app, edit_result, file_number, file_chosen;
  uint32_t send_buffer_len;
  char *sysex_save_filename; // max filename len is 16 chars
  char *filename_to_play;
  char *dirname;
  char *sysex_msg_info;
  char cwd[]= ".";
  scroll_list_item_t *file_list;

  G_active_app = APP_SYSEX_LIBRARIAN;

  do_exit = 0;
  file_chosen = 0;
  file_number = 0;
  lcd_needs_update = 1;
  sysex_save_filename = malloc(16);
  filename_to_play = NULL;
  sysex_msg_info = malloc(16);

  while(!do_exit)
   {

    // update main app screen

    if(lcd_needs_update)
     {
      LCDclear();
      IH_set_keymap_bar("REC","PLY","SAV","CLR");
      IH_set_status_bar(" SYSEX LIB    ");
      if(G_sysex_record_status == 0)
       LCDdrawstring(0,11,"   rec off", TEXT_NORMAL);
      else
       LCDdrawstring(0,11,"   rec on", TEXT_NORMAL);

      sprintf(sysex_msg_info," rcv:%2d rec:%2d",G_received_sysex_msg_count,G_saved_sysex_msg_count);
      LCDdrawstring(0,21,sysex_msg_info, TEXT_NORMAL);
      pthread_mutex_lock(&G_display_lock);
      LCDdisplay();
      pthread_mutex_unlock(&G_display_lock);
      lcd_needs_update = 0;
     }

    SYS_debug(DEBUG_NORMAL,"reading key events");

    read(G_keyboard_event_pipe[0],&key_event,1);

    SYS_debug(DEBUG_NORMAL,"key event: %d",key_event);

    // run main app loop and respond to key events

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
       lcd_needs_update = 1;
       break;

      case KEY2:
       file_chosen = 0;
       chdir(DEFAULT_SYSEX_DIR);

       while(!file_chosen)
        {
          file_list = IH_get_file_list(cwd);

          if(file_list == NULL)
           {
            IH_info("no files");
            lcd_needs_update = 1;
            break;
           }

          file_number = IH_scroll_list(file_list,"Choose file   ");

          if(file_number>0)
           {
             filename_to_play = IH_get_file_name_from_code(file_number,file_list);
           }
          else
           {
            lcd_needs_update = 1;
            break;
           }

          if(filename_to_play[0] == '/')
           {
             dirname = malloc(strlen(filename_to_play)-1);
             memcpy(dirname,filename_to_play+1,strlen(filename_to_play));
             chdir(dirname);
             free(dirname);
           }
          else
           file_chosen = 1;
        }

        if((filename_to_play == NULL) || (file_number == 0))
         {
          lcd_needs_update = 1;
          break;
         }

        send_buffer_len = SYS_read_sysex_buffer_from_file(filename_to_play,&G_sysex_transmit_buffer);

        if(G_sysex_transmit_buffer[0] != 0xF0) 
         {
           IH_info("wrong format!");
           free(filename_to_play);
           lcd_needs_update = 1;
           break;
         }

        if(send_buffer_len > 0)
           {
            LCDclear();
            IH_set_status_bar(" SYSEX SEND  ");
            IH_quick_info(" sending sysex");
            MIDI_write_sysex_buffer(G_sysex_transmit_buffer,send_buffer_len);
            IH_quick_info("  finished   ");
            sleep(1);
           }
          else
           {
            SYS_debug("SYS: read sysex file failed. Nothing sent to MIDI OUT.");
            IH_info("file read err");
           }
        free(filename_to_play);

        lcd_needs_update = 1;
       break;

      case KEY3:
       if(G_saved_sysex_msg_count == 0)
        {
         IH_info(" buffer empty");
         break;
        }
       memset(sysex_save_filename,32,16);
       sysex_save_filename[15] = 0x0;
       edit_result = IH_edit_string(&sysex_save_filename);
       if(edit_result == 1)
        {
          if(SYS_write_sysex_buffer_to_disk(G_received_sysex_msgs,G_saved_sysex_msg_count,sysex_save_filename) > 0)
           {
            IH_info(" save failed");
           }
          else
           {
            SYS_free_sysex_buffer();
            IH_info(" sysex saved");
           }
        }
        lcd_needs_update = 1;
       break;

      case KEY4:
       if(G_saved_sysex_msg_count >0)
         {
          SYS_free_sysex_buffer();
          SYS_debug(DEBUG_NORMAL,"SYS: sysex buffer discarded");
          G_saved_sysex_msg_count = 0;
         }
        lcd_needs_update = 1;
       break;

     } 
 
    //LCDclear();
    //pthread_mutex_lock(&G_display_lock);
    //LCDdisplay();
    //pthread_mutex_unlock(&G_display_lock);

   }
 
  return APP_SYSEX_LIBRARIAN;

 }

uint8_t IH_sequencer_app(void)
 {

   uint8_t lcd_needs_update;
   uint8_t do_exit;
   uint8_t key_event, next_app;

   lcd_needs_update = 1;
   do_exit = 0;

   G_active_app = APP_SEQUENCER;

   G_sequencer_state = SEQUENCER_STOP;

   while(!do_exit)
    {
      if(lcd_needs_update)
       {
         LCDclear();

         IH_set_keymap_bar("REC","PLY","EDT","CNF");
         IH_set_status_bar(" SEQUENCER    ");

         pthread_mutex_lock(&G_display_lock);
         LCDdisplay();
         pthread_mutex_unlock(&G_display_lock);
         lcd_needs_update = 0;
      }

    SYS_debug(DEBUG_NORMAL,"reading key events");

    read(G_keyboard_event_pipe[0],&key_event,1);

    SYS_debug(DEBUG_NORMAL,"key event: %d",key_event);

    switch(key_event)
     {

      case KEY1:
       G_sequencer_state = SEQUENCER_REC_STOP;
       SEQ_sequencer_record();
       lcd_needs_update = 1;
       break;
    
      case KEY2:
       SEQ_sequencer_play();
       lcd_needs_update = 1;
       break;

      case KEY3:
       SEQ_sequencer_edit_part();
       lcd_needs_update = 1;
       break;

      case KEY4:
       SEQ_sequencer_setup();
       lcd_needs_update = 1;
       break;

      case ENC_KEY:
       next_app = IH_choose_app();
       if(next_app != 0)
         return next_app;
       break;     

     }

   }

   return APP_SYSEX_LIBRARIAN;
 }

uint8_t IH_mididump_app(void)
 {
  uint8_t key_event, lcd_needs_update;
  uint8_t timer_command;
  uint8_t do_exit, next_app;
  uint16_t display_packet_index;
  char *dump_status;
  int error;

  lcd_needs_update = 1;
  do_exit = 0;
  display_packet_index = 1;
  dump_status = malloc(16);

  while(!do_exit)
   {

    // update main app screen

    if(lcd_needs_update)
     {
      LCDclear();
      if(!G_mididump_active)
       IH_set_keymap_bar("CAP","DET","CLR","OPT");
      else
       IH_set_keymap_bar("STP","DET","CLR","OPT");

      if(G_mididump_active)
        sprintf(dump_status," DUMP R %3d   ",G_mididump_packet_count);
      else
        sprintf(dump_status," DUMP S %3d   ",G_mididump_packet_count);

      IH_set_status_bar(dump_status);

      if(G_mididump_packet_count > 0)
       MD_display_packet(12,display_packet_index,G_mididump_packet_chain,TEXT_INVERTED);
      if((G_mididump_packet_count > 1) && (display_packet_index < G_mididump_packet_count))
       MD_display_packet(21,display_packet_index+1,G_mididump_packet_chain,TEXT_NORMAL);

      pthread_mutex_lock(&G_display_lock);
      LCDdisplay();
      pthread_mutex_unlock(&G_display_lock);
      lcd_needs_update = 0;
     }

    SYS_debug(DEBUG_NORMAL,"reading key events");

    read(G_keyboard_event_pipe[0],&key_event,1);

    SYS_debug(DEBUG_NORMAL,"key event: %d",key_event);

    // run main app loop and respond to key events

    switch(key_event)
     {

      case KEY1:
       if(G_mididump_active)
        {
          G_mididump_active = 0;
          timer_command = TIMER_STOP;
          write(G_MIDI_IN_timer_command_pipe[1],&timer_command,1);
        }
       else
        {
          G_mididump_active = 1;
          timer_command = TIMER_START;
          write(G_MIDI_IN_timer_command_pipe[1],&timer_command,1);
        }

       lcd_needs_update = 1;
       break;

      case ENC_KEY:
       next_app = IH_choose_app();
       if(next_app != 0)
         return next_app;
       break;

      case KEY_REFRESH_DISPLAY:
       if(G_mididump_packet_count > 1)
        display_packet_index = G_mididump_packet_count - 1;
       lcd_needs_update = 1;
       break;

      case KEY3:
       G_mididump_active = 0;
       G_mididump_packet_count = 0;
       display_packet_index = 1;
       timer_command = TIMER_RESET;
       write(G_MIDI_IN_timer_command_pipe[1],&timer_command,1);
       MD_destroy_packet_chain(G_mididump_packet_chain);
       G_mididump_packet_chain = NULL;
       lcd_needs_update = 1;
       break;

      case ENC_UP:
       if(display_packet_index > 1)
        display_packet_index--;
       lcd_needs_update = 1;
       break;

      case ENC_DOWN:
       if(display_packet_index < G_mididump_packet_count)
        display_packet_index++;
       lcd_needs_update = 1;
       break;

     }
   }

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
        break;

        case APP_SEQUENCER:
         next_app = IH_sequencer_app();
        break;

        case APP_MIDIDUMP:
         next_app = IH_mididump_app();
        break;

        case APP_ABOUT:
         next_app = IH_about_app();
        break;

       }
     
      sleep(1);

    }

 }

uint8_t IH_MIDI_inout_indicator()
 {

  uint8_t event_type;
  int flags;
  char  fake_buffer[4096];

  pipe(G_MIDI_inout_event_pipe);

  fcntl(G_MIDI_inout_event_pipe[0], F_SETPIPE_SZ, 4096);
  
  // flash MIDI in/out indicators when MIDI packets are going in/out of the device

  while(1)
   {
    event_type = 0;

    flags = fcntl(G_MIDI_inout_event_pipe[0], F_GETFL, 0); 
    flags |= O_NONBLOCK;
    fcntl(G_MIDI_inout_event_pipe[0], F_SETFL, flags);
    read(G_MIDI_inout_event_pipe[0],&fake_buffer,4096);  // flush the pipe by reading entire buffer at once

    flags &= ~O_NONBLOCK;
    fcntl(G_MIDI_inout_event_pipe[0], F_SETFL, flags);

    read(G_MIDI_inout_event_pipe[0],&event_type,1);  // now wait for incoming events

    if(event_type == MIDI_IN)
     {
       LCDdrawstring(78,1,"<",TEXT_INVERTED);
     }
    else if(event_type == MIDI_OUT)
     {
       LCDdrawstring(78,1,">",TEXT_INVERTED);
     }
    pthread_mutex_lock(&G_display_lock);
    LCDdisplay();
    pthread_mutex_unlock(&G_display_lock);
    usleep(130000);
    LCDdrawstring(78,1," ",TEXT_INVERTED);
    pthread_mutex_lock(&G_display_lock);
    LCDdisplay();
    pthread_mutex_unlock(&G_display_lock);
    usleep(150000);
   }

 }


