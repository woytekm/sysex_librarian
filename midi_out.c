#include "common.h"
#include "global.h"

uint8_t MIDI_write_sysex_buffer(unsigned char *sysex_buffer, uint32_t len)
 {

   #define SYSEX_CHUNK 16

   uint8_t buffer_empty, event_type;
   uint32_t offset = 0;
   uint32_t chunks,i;
   uint8_t partial_chunk;
   int MIDI_fd;
   mode_t mode;
   
   chunks = len/SYSEX_CHUNK;
   partial_chunk = len%SYSEX_CHUNK;


   SYS_debug(DEBUG_NORMAL,"SYS: writing MIDI SYSEX: chunks %d partial chunk %d.",chunks,partial_chunk);

   if((chunks == 0) && (partial_chunk > 0))
    {

     // tell appropriate thread to signal outgoing MIDI on the display
     event_type = MIDI_OUT;
     write(G_MIDI_inout_event_pipe[1],&event_type,1);

     write(G_MIDI_fd,sysex_buffer+offset,partial_chunk);
     return 1;
    }

   for(i=1; i<=chunks; i++)
    {

     // tell appropriate thread to signal outgoing MIDI on the display
     event_type = MIDI_OUT;
     write(G_MIDI_inout_event_pipe[1],&event_type,1);

     write(G_MIDI_fd,sysex_buffer+offset,SYSEX_CHUNK);
     offset += SYSEX_CHUNK;
     usleep(5000);
     //SYS_debug(DEBUG_NORMAL,"SYS: wrote chunk %d, (%d bytes)",i,offset);
    }

   if(partial_chunk > 0)
    {

     // tell appropriate thread to signal outgoing MIDI on the display
     event_type = MIDI_OUT;
     write(G_MIDI_inout_event_pipe[1],&event_type,1);

     write(G_MIDI_fd,sysex_buffer+offset,partial_chunk);
     SYS_debug(DEBUG_NORMAL,"SYS: wrote partial chunk %d, (%d bytes)",i,offset+partial_chunk);
    }

   return 1;

 }

uint8_t MIDI_write_short_event(midi_packet_t *event)
 {
     
     uint8_t event_type = MIDI_OUT;

     write(G_MIDI_inout_event_pipe[1],&event_type,1);
     if( write(G_MIDI_fd,event->packet_buffer,event->packet_len) < 0 )
      return 0;
     else
      return 1;
 }

uint8_t MIDI_all_notes_off_on_all_channels(void)           // some synths (as my Wavestation EX) apparently do not respont to "all notes off" CC in certain MIDI receive modes
 {                                                         // this function does "brute force" all notes off on all channels 
  uint8_t i,j;
  uint8_t note;
  uint8_t channel;
  midi_packet_t note_off;

  note_off.packet_len = 3;
  note_off.packet_buffer = malloc(3);
  note_off.next_packet = NULL;
  note_off.prev_packet = NULL;
  note_off.arrival_time = 0;
  note_off.packet_buffer[0] = 0x0;
  note_off.packet_buffer[1] = 0x0;
  note_off.packet_buffer[2] = 0x0;
  
  for(j=0; j < 16; j++)
   { 
    note_off.packet_buffer[0] = 0x90 | j;
    for(i=0; i<128; i++)
     {
      note_off.packet_buffer[1] = i;
      MIDI_write_short_event(&note_off);
     }
   }

  free(note_off.packet_buffer);
  return 0;

 }
