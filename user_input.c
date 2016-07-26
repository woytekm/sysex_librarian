#include "common.h"
#include "global.h"

void SYS_keyboard_thread(void)
 {

  uint8_t input;
  uint16_t msg_cnt;
  uint32_t total_bytes;
  uint32_t send_buffer_len;
 
   while(1)
    {
      input=wgetch(G_status_window);
 
      if(input == 'r')
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

