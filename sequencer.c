#include "common.h"
#include "global.h"

void SEQ_player(void *param)
 {

   uint8_t event_type, i, finished_tracks;
   uint32_t event_interval_counter;
   int flags, error;
   pipe(G_sequencer_player_command_pipe);
   midi_packet_t all_notes_off;

   midi_packet_t *current_packet_in_part[MAX_TRACKS];
   uint8_t        current_part_in_track[MAX_TRACKS];
   uint16_t       played_part_events[MAX_TRACKS];


   // build all notes off packet
   all_notes_off.packet_len = 3;
   all_notes_off.packet_buffer = malloc(3);
   all_notes_off.next_packet = NULL; 
   all_notes_off.prev_packet = NULL;
   all_notes_off.arrival_time = 0;
   all_notes_off.packet_buffer[0] = 0xB0;
   all_notes_off.packet_buffer[1] = 0x7B;
   all_notes_off.packet_buffer[2] = 0x0;

   while(1)
    {
       flags = fcntl(G_sequencer_player_command_pipe[0], F_GETFL, 0);
       flags &= ~O_NONBLOCK;
       fcntl(G_sequencer_player_command_pipe[0], F_SETFL, flags);
       read(G_sequencer_player_command_pipe[0],&event_type,1);    // blocking read, code will hang here until PLAYER_START command arrives

       if(event_type == SEQUENCER_PLAYER_START)
        {
          flags = fcntl(G_sequencer_player_command_pipe[0], F_GETFL, 0);
          flags |= O_NONBLOCK;
          fcntl(G_sequencer_player_command_pipe[0], F_SETFL, flags);

          G_sequencer_tick_interval = ((60000000/G_sequencer_BPM) / G_sequencer_PPQN) - TICK_INTERNAL_DELAY_IN_PLAY_LOOP;
   
          G_sequencer_ticks = 0;
          G_last_sequencer_event_time = 0;
          G_last_played_packet = NULL;

          // init sequencer data structures before playing

          for(i = 1; i < MAX_TRACKS; i++)
           {
            current_part_in_track[i] = 1;
            played_part_events[i] = 0;
            if(G_sequencer_tracks[i].parts[current_part_in_track[i]].event_count > 0)
              current_packet_in_part[i] = G_sequencer_tracks[i].parts[current_part_in_track[i]].first_packet;
            else
             {
              current_packet_in_part[i] = NULL;
             }
           }

          while((event_type != SEQUENCER_PLAYER_STOP) && (G_sequencer_state == SEQUENCER_PLAYING))
           {
            usleep(G_sequencer_tick_interval);
            G_sequencer_ticks++;

            finished_tracks = 0;

            for(i = 1; i < MAX_TRACKS; i++)
             {
              if(current_packet_in_part[i] == NULL)
               {
                finished_tracks++;
                continue;
               }
              if((current_packet_in_part[i]->arrival_time == (G_sequencer_ticks - G_last_sequencer_event_time)) &&
                 (G_sequencer_tracks[i].parts[current_part_in_track[i]].event_count > played_part_events[i]))
               {
                MIDI_write_short_event(current_packet_in_part[i]);
                G_last_sequencer_event_time = G_sequencer_ticks;
                played_part_events[i]++;

                if(current_packet_in_part[i]->packet_len == 2)
                  SYS_debug(DEBUG_HIGH,"play: packet [%x,%x] play time: %d",current_packet_in_part[i]->packet_buffer[0],current_packet_in_part[i]->packet_buffer[1],current_packet_in_part[i]->arrival_time);
                else if(current_packet_in_part[i]->packet_len == 3)
                  SYS_debug(DEBUG_HIGH,"play: packet [%x,%x,%x] play time: %d",current_packet_in_part[i]->packet_buffer[0],
                                                                              current_packet_in_part[i]->packet_buffer[1],
                                                                              current_packet_in_part[i]->packet_buffer[2],
                                                                              current_packet_in_part[i]->arrival_time);

                G_last_played_packet = current_packet_in_part[i];
                event_type = KEY_REFRESH_DISPLAY;
                write(G_keyboard_event_pipe[1],&event_type,1);

                if(played_part_events[i] < G_sequencer_tracks[i].parts[current_part_in_track[i]].event_count)
                 {
                   current_packet_in_part[i] = current_packet_in_part[i]->next_packet;  
                if(current_packet_in_part[i]->packet_len == 2)
                  SYS_debug(DEBUG_HIGH,"play: next packet [%x,%x] play time: %d",current_packet_in_part[i]->packet_buffer[0],current_packet_in_part[i]->packet_buffer[1],current_packet_in_part[i]->arrival_time);
                else if(current_packet_in_part[i]->packet_len == 3)
                  SYS_debug(DEBUG_HIGH,"play: next packet [%x,%x,%x] play time: %d",current_packet_in_part[i]->packet_buffer[0],
                                                                              current_packet_in_part[i]->packet_buffer[1],
                                                                              current_packet_in_part[i]->packet_buffer[2],
                                                                              current_packet_in_part[i]->arrival_time);
                 }
               }
              if(G_sequencer_tracks[i].parts[current_part_in_track[i]].end_time == G_sequencer_ticks)
               {
                 SYS_debug(DEBUG_NORMAL,"player: end of the part %d in track %d",current_part_in_track[i],i);
                 if(G_sequencer_tracks[i].part_count > current_part_in_track[i])
                  {
                   current_part_in_track[i]++;
                   current_packet_in_part[i] = G_sequencer_tracks[i].parts[current_part_in_track[i]].first_packet;
                   played_part_events[i] = 0;
                   SYS_debug(DEBUG_NORMAL,"player: start of the part %d in track %d",current_part_in_track[i],i);    
                  }
                 else
                  {
                   SYS_debug(DEBUG_NORMAL,"player: end of the track %d",i);
                   current_packet_in_part[i] = NULL;                           // terminate this track - all of the parts were played
                   all_notes_off.packet_buffer[1] = 0xB0 | (i - 1);            // set midi channel = track number (for now)
                   MIDI_write_short_event(&all_notes_off);
                  }
               }
             }

            if(finished_tracks == (MAX_TRACKS - 1)) // all tracks finished - stop the playback
             {
              SYS_debug(DEBUG_NORMAL,"\nplayer stop <-----------\n");
              G_sequencer_state = SEQUENCER_PLAY_STOP;
              event_type = KEY_REFRESH_DISPLAY;
              write(G_keyboard_event_pipe[1],&event_type,1);
             }

            read(G_sequencer_player_command_pipe[0],&event_type,1);

           }
        }
    }
 }




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
          G_sequencer_tick_interval = ((60000000/G_sequencer_BPM) / G_sequencer_PPQN) - TICK_INTERNAL_DELAY_IN_RECORD_LOOP;
 
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

void SEQ_init_tracks_and_parts()
 {
   uint8_t i,j;

  for(i = 0; i < MAX_TRACKS; i++)
   {
    G_sequencer_tracks[i].track_number = i;
    G_sequencer_tracks[i].part_count = 0;
    G_sequencer_tracks[i].MIDI_channel = i;
   
    for(j = 0; j < MAX_PARTS_PER_TRACK; j++)
     {
      G_sequencer_tracks[i].parts[j].part_number = j;
      G_sequencer_tracks[i].parts[j].start_time = 0;
      G_sequencer_tracks[i].parts[j].end_time = 0;
      G_sequencer_tracks[i].parts[j].event_count = 0;
      G_sequencer_tracks[i].parts[j].repeats = 0;
      G_sequencer_tracks[i].parts[j].packet_chain = NULL;
     }

   }

 }


void SEQ_initial_rec_LED_flash()
 {
  uint16_t tick_interval,ticks;
  uint8_t quaters;

  ticks = 0;
  quaters = 0;

  IH_set_status_bar(" SEQ REC PREP ");

  tick_interval = ((60000000/G_sequencer_BPM) / G_sequencer_PPQN) - TICK_INTERNAL_DELAY_IN_RECORD_LOOP;

  // 4/4 tempo, flash two bars (8 quaters), then start recording

  bcm2835_gpio_write(GPIO04_LED_PIN07,1);

  while(quaters < 8)
   {
    usleep(tick_interval);
    ticks++;
    if(ticks == 10)
     {
      bcm2835_gpio_write(GPIO04_LED_PIN07,0);
     }

    if(ticks == G_sequencer_PPQN) // one quater note has passed
     {
      bcm2835_gpio_write(GPIO04_LED_PIN07,1);
      ticks = 0;
      quaters++;
     }
    
   }

  bcm2835_gpio_write(GPIO04_LED_PIN07,0);

 }

void SEQ_choose_track_and_part()
 {

  uint8_t curr_track, curr_part;
  uint8_t do_exit, key_event, new_track, new_part, edit_row;
  uint8_t lcd_needs_update;
  char status[20];

  do_exit = 0;
  lcd_needs_update = 1;
  edit_row = 0;
  
  if(G_sequencer_tracks[G_current_track].parts[1].event_count == 0)
   new_part = 1;
  else
   new_part = 0;

  if(G_sequencer_tracks[1].part_count == 0)
   new_track = 1;
  else
   new_track = 0;


  while(!do_exit)
   {

    if(lcd_needs_update)
     {
      LCDclear();
      IH_set_status_bar(" SET TRK/PART ");
      IH_set_keymap_bar(" - "," + ","   ","OK ");

      if(G_sequencer_tracks[G_current_track].parts[G_current_part].event_count == 0)
       new_part = 1;
      else
       new_part = 0;

      if(G_sequencer_tracks[G_current_track].part_count == 0)
       new_track = 1;
      else
       new_track = 0; 
 
      if(new_track) 
       sprintf(status," TRACK:%2d NEW ",G_current_track);
      else
       sprintf(status," TRACK:%2d     ",G_current_track);

      if(edit_row == 0)
       LCDdrawstring(0,11,status, TEXT_INVERTED);
      else
       LCDdrawstring(0,11,status, TEXT_NORMAL);
    
      if(new_part)
       sprintf(status," PART: %2d NEW ", G_current_part);
      else
       sprintf(status," PART: %2d     ", G_current_part);
 
      if(edit_row == 1)
       LCDdrawstring(0,21,status, TEXT_INVERTED);
      else
       LCDdrawstring(0,21,status, TEXT_NORMAL);

      pthread_mutex_lock(&G_display_lock);
      LCDdisplay();
      pthread_mutex_unlock(&G_display_lock);

      lcd_needs_update = 0;
     }

  SYS_debug(DEBUG_NORMAL,"TRK/PART choose:  reading key events");

  read(G_keyboard_event_pipe[0],&key_event,1);

  SYS_debug(DEBUG_NORMAL,"key event: %d",key_event);

  switch(key_event)
    {

     case KEY2:
      if(edit_row == 0)
       {
       if((G_sequencer_tracks[G_current_track].part_count > 0) &&
          (G_current_track < MAX_TRACKS))
         {
           printf("debug: %d\n",G_sequencer_tracks[G_current_track].part_count);
           G_current_track++;  // allow for a new track if previous has anything on part 1
           G_current_part = 1;
         }
       }
      else if(edit_row == 1)
       {
       if( (G_sequencer_tracks[G_current_track].parts[G_current_part].event_count > 0) &&
           (G_current_part < MAX_PARTS_PER_TRACK) )
        G_current_part++;  // allow for a new part if previous has anything on it
       }
      lcd_needs_update = 1;
      break;

     case KEY1:
      if(edit_row == 0)
       {
        if(G_current_track > 1)
         {
           G_current_track--;  
           G_current_part=1;
         }
       }
      else if(edit_row == 1)
       {
         if(G_current_part > 1)
          G_current_part--;  
       }
      lcd_needs_update = 1;
      break;
 
     case KEY4:
      return;
      break;

     case ENC_UP:
      edit_row = 1 - edit_row;
      lcd_needs_update = 1;
      break;

     case ENC_DOWN:
      edit_row = 1 - edit_row;
      lcd_needs_update = 1;
      break;
      
    }

   }

 }


void SEQ_sequencer_record()
 {

  uint8_t key_event, lcd_needs_update;
  uint8_t timer_command;
  uint8_t do_exit, next_app;
  char seq_status[20];
  char track_part_info[20];
  int error;

  lcd_needs_update = 1;
  do_exit = 0;

  while(!do_exit)
   {

    if(lcd_needs_update)
     {
      LCDclear();
      if(G_sequencer_state == SEQUENCER_RECORDING)
       {
         IH_set_keymap_bar("STP","PSE","   ","   ");
         sprintf(seq_status," SEQ RECORDING");
         sprintf(track_part_info,"T:%d/P:%2d/E:%3d",G_current_track,G_current_part,G_sequencer_tracks[G_current_track].parts[G_current_part].event_count);
         LCDdrawstring(0,11,track_part_info, TEXT_NORMAL);
       }
      else if(G_sequencer_state == SEQUENCER_REC_PAUSED)
       {
         IH_set_keymap_bar("REC","   ","   ","EXI");
         sprintf(seq_status," SEQ PSE      ");
       }
      else if(G_sequencer_state == SEQUENCER_REC_STOP)
       {
         IH_set_keymap_bar("REC","SET","DEL","EXI");
         sprintf(seq_status," SEQ REC STOP ");
         sprintf(track_part_info,"T:%d/P:%2d/E:%3d",G_current_track,G_current_part,G_sequencer_tracks[G_current_track].parts[G_current_part].event_count);
         LCDdrawstring(0,11,track_part_info, TEXT_NORMAL);
       }

      IH_set_status_bar(seq_status);

      if(G_sequencer_tracks[G_current_track].parts[G_current_part].event_count > 0)
       MD_display_packet(21,G_sequencer_tracks[G_current_track].parts[G_current_part].event_count,
                          G_sequencer_tracks[G_current_track].parts[G_current_part].packet_chain,TEXT_INVERTED);

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
           if(G_sequencer_tracks[G_current_track].parts[G_current_part].event_count > 0)
            {
             if(!IH_yesno_dialog("  Overwrite? "))
              {
               lcd_needs_update = 1;
               break;
              }
             else
              {
               MD_destroy_packet_chain(G_sequencer_tracks[G_current_track].parts[G_current_part].packet_chain);
               G_sequencer_tracks[G_current_track].parts[G_current_part].packet_chain = NULL;
               G_sequencer_tracks[G_current_track].parts[G_current_part].event_count = 0; 
               G_sequencer_tracks[G_current_track].part_count--;
              }
            }
           SEQ_initial_rec_LED_flash();  
           G_sequencer_state = SEQUENCER_RECORDING;      

           if(G_sequencer_tracks[G_current_track].parts[G_current_part].part_number == 1)           
             G_sequencer_tracks[G_current_track].parts[G_current_part].start_time = 0;
           else
             G_sequencer_tracks[G_current_track].parts[G_current_part].start_time = G_sequencer_tracks[G_current_track].parts[G_current_part-1].end_time+1;

           timer_command = TIMER_START;
           write(G_MIDI_IN_timer_command_pipe[1],&timer_command,1);
         }
       else if(G_sequencer_state == SEQUENCER_RECORDING)
         {
           G_sequencer_state = SEQUENCER_REC_STOP;

           timer_command = TIMER_STOP;
           write(G_MIDI_IN_timer_command_pipe[1],&timer_command,1);

           G_sequencer_tracks[G_current_track].parts[G_current_part].end_time = G_sequencer_ticks;
 
           timer_command = TIMER_RESET;
           write(G_MIDI_IN_timer_command_pipe[1],&timer_command,1);

           if(G_sequencer_tracks[G_current_track].parts[G_current_part].event_count > 0)
            if((G_sequencer_tracks[G_current_track].part_count+1) == G_current_part)  
             G_sequencer_tracks[G_current_track].part_count++;    // we have new part with some data on it 
         }
        lcd_needs_update = 1;
       break;

      case KEY2:
       SEQ_choose_track_and_part();
       lcd_needs_update = 1;
      break;

      case KEY3:
       if(G_sequencer_tracks[G_current_track].parts[G_current_part].event_count > 0)
        {
           if(!IH_yesno_dialog(" delete part? "))
              {
               lcd_needs_update = 1;
               break;
              }
           else
             {
               MD_destroy_packet_chain(G_sequencer_tracks[G_current_track].parts[G_current_part].packet_chain);
               G_sequencer_tracks[G_current_track].parts[G_current_part].packet_chain = NULL;
               G_sequencer_tracks[G_current_track].parts[G_current_part].event_count = 0;
               G_sequencer_tracks[G_current_track].part_count--;
             }
        }
       else
        IH_info(" empty part  ");
       lcd_needs_update = 1;
      break;

      case KEY4:
       if((G_sequencer_state == SEQUENCER_RECORDING) || 
          (G_sequencer_state == SEQUENCER_REC_PAUSED) )
        G_sequencer_state = SEQUENCER_REC_STOP;
       return;
      break;
 
      case KEY_REFRESH_DISPLAY:
       lcd_needs_update = 1;
      break;

     } // switch end

   } // while(!do_exit) end


}

void SEQ_sequencer_play()
 {
  uint8_t key_event, lcd_needs_update;
  uint8_t player_command;
  uint8_t do_exit, next_app;
  char seq_status[20];

  lcd_needs_update = 1;
  do_exit = 0;

  G_sequencer_state = SEQUENCER_PLAY_STOP;

  while(!do_exit)
   {

    if(lcd_needs_update)
     {
      LCDclear();

      if(G_sequencer_state == SEQUENCER_PLAYING)
       {
         if(key_event == KEY_REFRESH_DISPLAY)
          {
           if(G_last_played_packet != NULL)
             MD_display_packet(21,G_last_played_packet->packet_id,G_last_played_packet,TEXT_INVERTED);
          }
         else
          {
           IH_set_keymap_bar("STP","PSE","   ","   ");
           sprintf(seq_status," SEQ PLAYING ");
           IH_set_status_bar(seq_status);
          }
       }
      else if(G_sequencer_state == SEQUENCER_PLAY_PAUSED)
       {
         IH_set_keymap_bar("PLY","   ","   ","EXI");
         sprintf(seq_status," SEQ PSE      ");
         IH_set_status_bar(seq_status);
       }
      else if(G_sequencer_state == SEQUENCER_PLAY_STOP)
       {
         IH_set_keymap_bar("PLY","SET","DEL","EXI");
         sprintf(seq_status," SEQ PLAY STOP");
         IH_set_status_bar(seq_status);
       }

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
       if((G_sequencer_state == SEQUENCER_PLAY_STOP) || (G_sequencer_state == SEQUENCER_PLAY_PAUSED))
        {
         G_sequencer_state = SEQUENCER_PLAYING;
         player_command = SEQUENCER_PLAYER_START;
         write(G_sequencer_player_command_pipe[1],&player_command,1);       
        }
       else if(G_sequencer_state == SEQUENCER_PLAYING)
        {
         G_sequencer_state = SEQUENCER_PLAY_STOP;
         player_command = SEQUENCER_PLAYER_STOP;
         write(G_sequencer_player_command_pipe[1],&player_command,1);
        }
        lcd_needs_update = 1;
       break;

      case KEY2:
       break;

      case KEY3:
       break;

      case KEY4:
       if((G_sequencer_state == SEQUENCER_PLAYING) ||
          (G_sequencer_state == SEQUENCER_PLAY_PAUSED) )
        G_sequencer_state = SEQUENCER_PLAY_STOP;
       return;
      break;

     case KEY_REFRESH_DISPLAY:
      lcd_needs_update = 1;
     break;

    }

  }

 }

void SEQ_sequencer_edit_part()
 {}

void SEQ_sequencer_setup()
 {}


