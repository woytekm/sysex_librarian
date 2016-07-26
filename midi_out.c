#include "common.h"
#include "global.h"

uint8_t MIDI_write_sysex_buffer(unsigned char *sysex_buffer, uint32_t len)
 {

   #define SYSEX_CHUNK 16

   uint8_t buffer_empty;
   uint32_t offset = 0;
   uint32_t chunks,i;
   uint8_t partial_chunk;
   int MIDI_fd;
   mode_t mode;
   
   char *test = "sdfsdfdsf";

   mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

   MIDI_fd = open("sysex_dump_compare.bin",O_WRONLY|O_CREAT|O_TRUNC,mode);

   if(MIDI_fd == -1)
    {
     SYS_debug(DEBUG_NORMAL,"SYS: open file failed, errno %d",errno);
     return;
    }

   chunks = len/SYSEX_CHUNK;
   partial_chunk = len%SYSEX_CHUNK;


   SYS_debug(DEBUG_NORMAL,"chunks, partial chunk: %d, %d",chunks,partial_chunk);

   if((chunks == 0) && (partial_chunk > 0))
    {
     write(MIDI_fd,sysex_buffer+offset,partial_chunk);
     close(MIDI_fd);
     return 1;
    }

   for(i=1; i<=chunks; i++)
    {
     write(MIDI_fd,sysex_buffer+offset,SYSEX_CHUNK);
     offset += SYSEX_CHUNK;
     usleep(5000);
     SYS_debug(DEBUG_NORMAL,"SYS: wrote chunk %d, (%d bytes)",i,offset);
    }

   if(partial_chunk > 0)
    {
     write(MIDI_fd,sysex_buffer+offset,partial_chunk);
     SYS_debug(DEBUG_NORMAL,"SYS: wrote partial chunk %d, (%d bytes)",i,offset+partial_chunk);
    }

 
   close(MIDI_fd);

   return 1;

 }
