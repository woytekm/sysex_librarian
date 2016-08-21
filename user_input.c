#include "common.h"
#include "global.h"


#ifdef IFACE_HW

uint8_t decode_encoder_event(uint8_t now, uint8_t prev, uint8_t prev_prev)
 {
   if( (now == 1) && (prev == 3) && (prev_prev == 2) )
    return ENC_DOWN;
   else if( (now == 3) && (prev == 2) && (prev_prev == 0) )
    return ENC_DOWN;
   else if( (now == 0) && (prev == 1) && (prev_prev == 3) )
    return ENC_DOWN;
   else if((now == 0) && (prev == 2) && (prev_prev == 3) )
    return ENC_UP;
   else if( (now == 3) && (prev == 1) && (prev_prev == 0) )
    return ENC_UP;
   else if((now == 2) && (prev == 3) && (prev_prev == 1) )
    return ENC_UP;

  return 0;

 }

void SYS_shiftin_thread(void *params)

{

 uint8_t shift_val_new,shift_val_prev;
 uint8_t encval, encval_prev, encval_prev_prev, event;
 uint8_t reg_bits;

 shift_val_new = shift_val_prev = 0;
 encval = encval_prev = encval_prev_prev = 0;
 

 pipe(G_keyboard_event_pipe);

 while(1)
  {

   shift_val_new = 0;

   bcm2835_gpio_clr(GPIO27_CP_PIN13);

   bcm2835_gpio_clr(GPIO17_PL_PIN11);
   usleep(10);
   bcm2835_gpio_set(GPIO17_PL_PIN11);

   bcm2835_gpio_clr(GPIO22_CE_PIN15);


   for(reg_bits=0; reg_bits<=7; reg_bits++)
    {
      bcm2835_gpio_set(GPIO27_CP_PIN13);
      shift_val_new |= bcm2835_gpio_lev(GPIO23_Q7_PIN16) << reg_bits;
      usleep(40);
      bcm2835_gpio_clr(GPIO27_CP_PIN13);
    }

   if(shift_val_new != shift_val_prev)
    {

     event = 0;

     if((shift_val_new & 64) == 0)
      event = KEY1;
     else if((shift_val_new & 32) == 0)
      event = KEY2;
     else if((shift_val_new & 16) == 0)
      event = KEY3;
     else if((shift_val_new & 8) == 0)
      event = KEY4;
     else if((shift_val_new & 1) == 0)
      event = ENC_KEY;

     if((event == KEY1) || (event == KEY2) || (event == KEY3) || (event == KEY4) || (event == ENC_KEY))
       write(G_keyboard_event_pipe[1],&event,1);
     else
      {
       encval_prev_prev = encval_prev;
       encval_prev = encval;
       encval = 0;

       if((shift_val_new & 4) == 0)
        encval |= 2;
       if((shift_val_new & 2) == 0)
        encval |= 1;

       if(encval != encval_prev)
        {
          event = decode_encoder_event(encval, encval_prev, encval_prev_prev);
          write(G_keyboard_event_pipe[1],&event,1);
        }
       }

    }

   shift_val_prev = shift_val_new;

   usleep(SHIFTIN_DELAY);

  }

}

#endif

#ifdef IFACE_CURSES

void SYS_keyboard_thread(void *params)
 {

  uint8_t input;
  uint16_t msg_cnt;
  uint32_t total_bytes;
  uint32_t send_buffer_len;

   if(G_use_iface_curses == 1)
    {
     while(1)
     {
      input=wgetch(G_status_window);
 
      if(input == 'r')  
       {
        if(G_sysex_record_status == 0)
        {
         G_sysex_record_status = 1;
         SYS_debug(DEBUG_NORMAL,"SYS: sysex recording turned on");
        }
       else 
        {
         G_sysex_record_status = 0;
         SYS_debug(DEBUG_NORMAL,"SYS: sysex recording turned off");
        }
       }

      if(input == 's')
       {
        send_buffer_len = SYS_read_sysex_buffer_from_file("sysex_dump.bin",&G_sysex_transmit_buffer);
        if(send_buffer_len > 0)
         MIDI_write_sysex_buffer(G_sysex_transmit_buffer,send_buffer_len);
        else
         SYS_debug("SYS: read sysex file failed. Nothing sent to MIDI OUT.");
       }

      if(input == 'w')
       {

        if(G_saved_sysex_msg_count >0)
         {
          total_bytes = 0;
          SYS_write_sysex_buffer_to_disk(G_received_sysex_msgs,G_saved_sysex_msg_count);
          for(msg_cnt=1; msg_cnt <= G_saved_sysex_msg_count; msg_cnt++)
           {
            free(G_received_sysex_msgs[msg_cnt].message);
            total_bytes += G_received_sysex_msgs[msg_cnt].length;
            G_received_sysex_msgs[msg_cnt].length = 0;
           }
          
          SYS_debug(DEBUG_NORMAL,"SYS: wrote sysex buffer to disk (%d messages, %d bytes total)",G_received_sysex_msg_count,total_bytes);
          G_saved_sysex_msg_count = 0;
         }
        else
         SYS_debug(DEBUG_NORMAL,"SYS: nothing to write! No received sysex msgs!");

       }

      if(input == 'd')
       {

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

       }

      if(input == 'q')
       {
        endwin();
        exit(0);
       }
     }
   }
   // if(start_curses_iface == 1)

 }

#endif


