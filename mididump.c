#include "common.h"
#include "global.h"


midi_packet_t *MD_add_packet_to_chain(void *buffer_from, uint32_t packet_len, midi_packet_t *last_packet)
 {

   midi_packet_t *new_packet;
   
   new_packet = malloc(sizeof(midi_packet_t));
   new_packet->packet_buffer = malloc(packet_len);

   memcpy(new_packet->packet_buffer,buffer_from,packet_len);
   
   if(last_packet != NULL)
     last_packet->next_packet = new_packet;

   new_packet->prev_packet = last_packet;
   new_packet->next_packet = NULL;
   new_packet->packet_len = packet_len;
    
   return new_packet;

 }

midi_packet_t *MD_find_packet_in_chain(midi_packet_t *packet_chain, uint16_t packet_id)
 {
  midi_packet_t *packet;

  packet = packet_chain;

  while(packet != NULL)
   {
     if(packet->packet_id == packet_id)
      return packet;
     packet = packet->prev_packet;
   }

  return NULL;
 }

void MD_display_packet(uint8_t display_row, uint16_t packet_id, midi_packet_t *packet_chain)
 {
  midi_packet_t *packet_to_display;
  uint8_t midi_channel, midi_msgtype;
  char *packet_info;
 
  packet_info = malloc(16); 

  packet_to_display = MD_find_packet_in_chain(packet_chain, packet_id);

  LCDdrawstring(0,display_row,"              ",TEXT_NORMAL);

  packet_to_display = MD_find_packet_in_chain(packet_chain, packet_id);

  if(packet_to_display->packet_buffer[0] < 240)  /* channel related message - let's split channel number and message type */
    {
     midi_channel = (packet_to_display->packet_buffer[0] & 0b00001111) + 1;  /* channel ID in MIDI messages starts from 0  */
     midi_msgtype = packet_to_display->packet_buffer[0] & 0b11110000;
    }
   else   /* message = 0b1111nnnn  - system common message or realtime message */
    {
     midi_channel = 0;
     midi_msgtype = packet_to_display->packet_buffer[0];
    }

  switch(midi_msgtype)
   {
     case 0x90:  /* note on */
     if(packet_to_display->packet_buffer[2] == 0; // note off
      sprintf(packet_info,"%2d NOTE OFF %2d",midi_channel,packet_to_display->packet_buffer[1]);
     else
      sprintf(packet_info,"%2d NOTE ON %2d %2d",midi_channel,packet_to_display->packet_buffer[1], packet_to_display->packet_buffer[2]);

     LCDdrawstring(0,display_row,packet_info,TEXT_NORMAL);

     pthread_mutex_lock(&G_display_lock);
     LCDdisplay();
     pthread_mutex_unlock(&G_display_lock);

     break;
   }

 free(packet_info);

 }
