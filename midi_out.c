#include "common.h"
#include "global.h"

uint8_t MIDI_write_sysex_buffer(unsigned char *sysex_buffer, uint32_t len)
 {

   #define SYSEX_CHUNK 8

   uint8_t buffer_empty;
   uint32_t offset = 0;
   uint32_t chunks,i;
   uint8_t partial_chunk;
   int MIDI_fd;
   mode_t mode;
   
   chunks = len/SYSEX_CHUNK;
   partial_chunk = len%SYSEX_CHUNK;


   SYS_debug(DEBUG_NORMAL,"chunks, partial chunk: %d, %d",chunks,partial_chunk);

   if((chunks == 0) && (partial_chunk > 0))
    {
     write(G_MIDI_fd,sysex_buffer+offset,partial_chunk);
     return 1;
    }

   for(i=1; i<=chunks; i++)
    {
     write(G_MIDI_fd,sysex_buffer+offset,SYSEX_CHUNK);
     offset += SYSEX_CHUNK;
     usleep(10000);
     SYS_debug(DEBUG_NORMAL,"SYS: wrote chunk %d, (%d bytes)",i,offset);
    }

   if(partial_chunk > 0)
    {
     write(G_MIDI_fd,sysex_buffer+offset,partial_chunk);
     SYS_debug(DEBUG_NORMAL,"SYS: wrote partial chunk %d, (%d bytes)",i,offset+partial_chunk);
    }

   return 1;

 }
