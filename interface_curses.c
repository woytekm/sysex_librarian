#include "common.h"
#include "global.h"


void SYS_update_status_window(void)
 {
      if(G_sysex_record_status == 0)
         mvwprintw(G_status_window,1,2,"Recording status: recording stopped");
      else if(G_sysex_record_status == 1)
         mvwprintw(G_status_window,1,2,"Recording status: recording started");
 
      mvwprintw(G_status_window,2,2,"Received sysex messages: %d",G_received_sysex_msg_count);
      mvwprintw(G_status_window,3,2,"Sysex messages currently in buffer: %d",G_saved_sysex_msg_count);

      mvwprintw(G_status_window,4,2,"Pendrive not connected");
      wrefresh(G_status_window);
 }


void SYS_init_curses_interface(void)
 {

   uint8_t row,col;

   initscr();
   noecho();
 
   getmaxyx(stdscr,row,col);
 
   if((col < PANEL_WIDTH) || (row < PANEL_HEIGHT))
    {
     endwin();
     printf("error - terminal to small\n");
     exit(-1);
    }
 
   G_menu_bar = newwin(1,PANEL_WIDTH,30,1);
   G_log_window = newwin(8,PANEL_WIDTH-2,21,2);
   G_status_window = newwin(16,PANEL_WIDTH-2,2,2);
 
 
   G_border_status_window = newwin(18,PANEL_WIDTH,1,1);
   G_border_log_window = newwin(10,PANEL_WIDTH,20,1);
 
   scrollok(G_log_window,TRUE);
 
   box(G_border_log_window,0,0);
   box(G_border_status_window,0,0);
 
   wrefresh(G_border_log_window);
   wrefresh(G_border_status_window);
 
   wprintw(G_menu_bar,"[r - start/stop record] [s - send SYX] [w - write to file] [d - discard buffer] [q - quit]");
 
   wrefresh(G_menu_bar); 


 }

