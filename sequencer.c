#include "common.h"
#include "global.h"

void SYS_MIDI_IN_timer(void *param)
{
 
   uint8_t event_type;
   int flags,error;
 
   pipe(G_MIDI_IN_timer_command_pipe);

   while(1)
     {
       flags = fcntl(G_MIDI_IN_timer_command_pipe[0], F_GETFL, 0);
       flags &= ~O_NONBLOCK;
       fcntl(G_MIDI_IN_timer_command_pipe[0], F_SETFL, flags);
       read(G_MIDI_IN_timer_command_pipe[0],&event_type,1);
 
       // in loop, we will wait for timer start command (blocking read), and then we will do non blocking
       // checks on command pipe descriptor for timer stop command
 
       if(event_type == TIMER_START)
         {
 
          flags = fcntl(G_MIDI_IN_timer_command_pipe[0], F_GETFL, 0);
          flags |= O_NONBLOCK;
          fcntl(G_MIDI_IN_timer_command_pipe[0], F_SETFL, flags);
 
          // calculated in microseconds:
          G_sequencer_tick_interval = ((60000000/G_sequencer_BPM) / G_sequencer_PPQN) - TICK_INTERNAL_DELAY;
 
          SYS_debug(DEBUG_NORMAL,"MIDI IN timer start. Tick interval: %d", G_sequencer_tick_interval);

          G_sequencer_ticks = 0;
          G_last_sequencer_event_time = 0;
 
          while(event_type != TIMER_STOP)
           {
            usleep(G_sequencer_tick_interval);
            G_sequencer_ticks++;
            read(G_MIDI_IN_timer_command_pipe[0],&event_type,1);
           } 

          SYS_debug(DEBUG_NORMAL,"MIDI IN timer stop. %d ticks counted", G_sequencer_ticks);     
 
         }
 
       if(event_type == TIMER_RESET)
         {
           G_sequencer_ticks = 0;
           G_last_sequencer_event_time = 0;
         }
     }
 }

void SEQ_initial_rec_LED_flash()
 {}

void SEQ_sequencer_record()
 {

  uint8_t key_event, lcd_needs_update;
  uint8_t timer_command;
  uint8_t do_exit, next_app;
  uint16_t display_packet_index;
  char *seq_status;
  int error;

  lcd_needs_update = 1;
  do_exit = 0;
  display_packet_index = 1;
  seq_status = malloc(16);

  while(!do_exit)
   {

    if(lcd_needs_update)
     {
      LCDclear();
      if(G_sequencer_state == SEQUENCER_RECORDING)
       {
         IH_set_keymap_bar("STP","PSE","   ","   ");
         sprintf(seq_status," SEQ REC %3d  ",G_mididump_packet_count);
       }
      else if(G_sequencer_state == SEQUENCER_REC_PAUSE)
       {
         IH_set_keymap_bar("REC","   ","   ","EXI");
         sprintf(seq_status," SEQ PSE %3d  ",G_mididump_packet_count);
       }
      else if(G_sequencer_state == SEQUENCER_REC_STOP)
       {
         IH_set_keymap_bar("REC","   ","   ","EXI");
         sprintf(seq_status," SEQ REC %3d  ",G_mididump_packet_count);
       }

      IH_set_status_bar(seq_status);

      if(G_sequencer_tracks[G_current_track].parts[G_current_part].event_count > 0)
       MD_display_packet(12,display_packet_index,G_sequencer_tracks[G_current_track].parts[G_current_part].packet_chain,TEXT_INVERTED);
      if((G_sequencer_tracks[G_current_track].parts[G_current_part].event_count > 1) && (display_packet_index < G_sequencer_tracks[G_current_track].parts[G_current_part].event_count))
       MD_display_packet(21,display_packet_index+1,G_sequencer_tracks[G_current_track].parts[G_current_part].packet_chain,TEXT_NORMAL);

      pthread_mutex_lock(&G_display_lock);
      LCDdisplay();
      pthread_mutex_unlock(&G_display_lock);
      lcd_needs_update = 0;
     }

  SYS_debug(DEBUG_NORMAL,"SEQ REC reading key events");

  read(G_keyboard_event_pipe[0],&key_event,1);

  SYS_debug(DEBUG_NORMAL,"key event: %d",key_event);


  switch(key_event)
    {

      case KEY1:
       if(G_sequencer_state == SEQUENCER_REC_STOP)
         {
           SEQ_initial_rec_LED_flash();  
           G_sequencer_state = SEQUENCER_RECORDING;                 
           timer_command = TIMER_START;
           write(G_MIDI_IN_timer_command_pipe[1],&timer_command,1);
         }
       else if(G_sequencer_state == SEQUENCER_RECORDING)
         {
           G_sequencer_state = SEQUENCER_REC_STOP;
           timer_command = TIMER_STOP;
           write(G_MIDI_IN_timer_command_pipe[1],&timer_command,1);
         }
        lcd_needs_update = 1;
       break;

      case KEY4:
       if((G_sequencer_state == SEQUENCER_RECORDING) || 
          (G_sequencer_state == SEQUENCER_REC_PAUSE) )
        G_sequencer_state = SEQUENCER_REC_STOP;

        return;
       break;

     } // switch end

   } // while(!do_exit) end


}
void SEQ_sequencer_play()
 {}

void SEQ_sequencer_edit_part()
 {}

void SEQ_sequencer_setup()
 {}


