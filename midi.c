#include "global.h"
#include "common.h"

uint32_t MIDI_get_sysex_len(unsigned char *buffer, uint32_t buflen)
 {

  #define CHECK_BOUNDARY 512 // we will check last CHECK_BOUNDARY bytes of buffer for 0xF7 sysex end messsag

  uint32_t count = buflen - 1;

  //if(buffer[0] != 0xF0) return 0;

  while((buffer[count] != 0xF7) && (count >= 0) && ((buflen - count) < CHECK_BOUNDARY))
   count--;

  if(buffer[count] == 0xF7)
     return count+1;
  else
    return 0;
 }


uint8_t MIDI_is_partial_message(unsigned char *buffer, uint32_t len)
 {
  uint32_t buffer_position, sysex_len;
  uint8_t finished, midi_msgtype;

  buffer_position = 0;
  finished = 0;

  while(!finished)
   {
     if(buffer[buffer_position] < 240)  /* channel related message - let's split channel number and message type */
       midi_msgtype = buffer[buffer_position] & 0b11110000;
     else   /* message = 0b1111nnnn  - system common message or realtime message */
       midi_msgtype = buffer[buffer_position];

     if((G_MIDI_msg_lengths[midi_msgtype] == 0) && (midi_msgtype != 0xF0))
      return 0;  /* something is wrong - data seems to be corrupted - return and let upper layers take care of this */
     
     if(midi_msgtype == 0xF0)   /* sysex - variable lenghth */
      {   
       sysex_len = MIDI_get_sysex_len(buffer, len);
       if(sysex_len == 0) 
        return 1; /* incomplete sysex message */
       else
        buffer_position += sysex_len;
      }
     else
      buffer_position += G_MIDI_msg_lengths[midi_msgtype];
 
     if(buffer_position > len) return 1;
     if(buffer_position == len) return 0;
   }
  
  return 0;

 }


void MIDI_init_MIDI_msg_lenghts(void)
 {

  G_MIDI_msg_lengths[0b10000000] = 3;   /* note off    */
  G_MIDI_msg_lengths[0b10010000] = 3;   /* note on     */
  G_MIDI_msg_lengths[0b10100000] = 3;   /* aftertouch  */
  G_MIDI_msg_lengths[0b10110000] = 3;   /* control change */
  G_MIDI_msg_lengths[0b11000000] = 2;   /* pgm change  */
  G_MIDI_msg_lengths[0b11010000] = 2;   /* aftertouch  */
  G_MIDI_msg_lengths[0b11100000] = 3;   /* pitch bend  */
  G_MIDI_msg_lengths[0b11110000] = 0;   /* sysex - undefined */
  G_MIDI_msg_lengths[0b11111000] = 1;   /* MIDI clock */
  G_MIDI_msg_lengths[0b11110001] = 1;   /* various common system/realtime messages: */
  G_MIDI_msg_lengths[0b11110010] = 1;
  G_MIDI_msg_lengths[0b11110011] = 1;
  G_MIDI_msg_lengths[0b11110110] = 1;
  G_MIDI_msg_lengths[0b11111010] = 1;
  G_MIDI_msg_lengths[0b11111011] = 1;
  G_MIDI_msg_lengths[0b11111110] = 1;
  G_MIDI_msg_lengths[0b11111111] = 1;   /* MIDI reset */

 }


 void MIDI_parse_msgbuffer(unsigned char *midi_in_buffer, uint32_t at_offset, uint32_t buflen)
  {

    uint8_t midi_channel, midi_msgtype, event_type;
    uint32_t next_message_offset = 0, sysex_len, msg_cnt;

    if(at_offset >= buflen)   /* end of buffer  */
     return;

#ifdef IFACE_HW
     // tell appropriate thread to signal incoming MIDI on the display
     if(G_use_iface_hw == 1)
      {
        event_type = MIDI_IN;
        write(G_MIDI_inout_event_pipe[1],&event_type,1);
      }
#endif

    if(midi_in_buffer[at_offset] < 240)  /* channel related message - let's split channel number and message type */
     {
      midi_channel = (midi_in_buffer[at_offset] & 0b00001111) + 1;  /* channel ID in MIDI messages starts from 0  */
      midi_msgtype = midi_in_buffer[at_offset] & 0b11110000;
     }
    else   /* message = 0b1111nnnn  - system common message or realtime message */
     {
      midi_channel = 0;
      midi_msgtype = midi_in_buffer[at_offset];
     }

    switch(midi_msgtype) {

     case 0x90:  /* note on */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0x90];

      if(midi_in_buffer[at_offset+2] == 0)   /* attack velocity = 0, this is NOTE OFF  */
        SYS_debug(DEBUG_HIGH,"RCV: MIDI note on (off), CH%d, (%x, %x)\n", midi_channel, midi_in_buffer[at_offset+1], midi_in_buffer[at_offset+2]);
      else
        SYS_debug(DEBUG_HIGH,"RCV: MIDI note on, CH%d, (%x, %x)\n", midi_channel, midi_in_buffer[at_offset+1], midi_in_buffer[at_offset+2]);

      if(midi_in_buffer[at_offset+1] > 95 )
       SYS_debug(DEBUG_HIGH,"  invalid MIDI note value %d!\n",midi_in_buffer[at_offset+1]);
      break;

     case 0x80:  /* note off  */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0x80];
      SYS_debug(DEBUG_HIGH,"RCV: MIDI note off, CH%d, (%x)\n", midi_channel, midi_in_buffer[at_offset+1]);
      break;

     case 0xA0:  /* aftertouch */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0xA0];
      SYS_debug(DEBUG_HIGH,"RCV: aftertouch message\n");
      break;

     case 0xB0:  /* control change */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0xB0];
      SYS_debug(DEBUG_HIGH,"RCV: control change message\n");
      break;

     case 0xC0: /* program change */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0xC0];
      SYS_debug(DEBUG_HIGH,"RCV: program change message\n");
      break;

     case 0xD0: /* aftertouch */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0xD0];
      SYS_debug(DEBUG_HIGH,"RCV: MIDI aftertouch message\n");
      break;

     case 0xE0: /* pitchbend */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0xE0];
      SYS_debug(DEBUG_HIGH,"RCV: MIDI pitchbend message\n");
      break;

     case 0xF0:  /* sysex */
      sysex_len = MIDI_get_sysex_len(&midi_in_buffer[at_offset], buflen - at_offset);

      G_received_sysex_msg_count++;

      SYS_debug(DEBUG_NORMAL,"RCV: MIDI system exclusive message number %d (%d bytes)", G_received_sysex_msg_count, sysex_len);

      if(G_received_sysex_msg_count == MAX_SYSEX_MSGS)
       break;

      if(!G_sysex_record_status) // recording stopped
       {
        SYS_debug(DEBUG_NORMAL,"RCV: dropping sysex message because sysex recording is turned off");
        next_message_offset = at_offset + sysex_len;
        break;
       }

      G_saved_sysex_msg_count++;
      G_received_sysex_msgs[G_saved_sysex_msg_count].message = malloc(sysex_len);
      memcpy((void *)G_received_sysex_msgs[G_saved_sysex_msg_count].message, (void *)&midi_in_buffer[at_offset], sysex_len);
      G_received_sysex_msgs[G_saved_sysex_msg_count].length = sysex_len;

      if(sysex_len == 0)   /* MIDI_get_sysex_len detected incomplete sysex message  - drop it */
       return;

      next_message_offset = at_offset + sysex_len;
      break;

     case 0xF8: /* MIDI clock */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0xF8];
      /* do not log this as it would overwhelm the program - too many msgs */
      break;
 
     case 0xFE: /* MIDI active sensing */
      next_message_offset = at_offset + 1; // MIDI active sensing length is 1
      /* do not log this as it would overwhelm the program - too many msgs */
      break;


     case 0xFF: /* MIDI reset */
      next_message_offset = at_offset + G_MIDI_msg_lengths[0xFF];
      SYS_debug(DEBUG_HIGH,"RCV: general MIDI reset message\n");
      break;

     default:
      if(midi_msgtype > 240)
       {
        next_message_offset = at_offset + 1;
        SYS_debug(DEBUG_HIGH,"RCV: unknown general system message (%x)\n",midi_msgtype);
       }
      else
       {
        SYS_debug(DEBUG_HIGH,"RCV: unsupported MIDI message or garbage - discarding message buffer\n");
        return;    
       }

   }

   MIDI_parse_msgbuffer(midi_in_buffer, next_message_offset, buflen);

 }

