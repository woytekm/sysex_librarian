#include "common.h"
#include "global.h"


void IH_startup_display(uint8_t about)
 {
    char *msgbuf;

    msgbuf = malloc(16);

    LCDclear();
    sprintf(msgbuf," version %d.%d",VER_MAJ,VER_MIN);
    if(about)
     {
       LCDdrawstring(0,14,"  MIDI cube  ", TEXT_NORMAL);
       LCDdrawstring(0,25,msgbuf,TEXT_NORMAL);
     }
    else
     {
       LCDdrawstring(0,10,"  MIDI cube  ", TEXT_NORMAL);
       LCDdrawstring(0,21,msgbuf,TEXT_NORMAL);
     }

    pthread_mutex_lock(&G_display_lock);
    LCDdisplay();
    pthread_mutex_unlock(&G_display_lock);

    free(msgbuf);

 }

void IH_set_keymap_bar(char *key1, char *key2, char *key3, char *key4)
 {
   if(strlen(key1))
     LCDdrawstring(0,36,key1,TEXT_INVERTED);
   if(strlen(key2))
     LCDdrawstring(22,36,key2,TEXT_INVERTED);
   if(strlen(key3))
     LCDdrawstring(44,36,key3,TEXT_INVERTED);
   if(strlen(key4))
     LCDdrawstring(65,36,key4,TEXT_INVERTED);

   pthread_mutex_lock(&G_display_lock);
   LCDdisplay();
   pthread_mutex_unlock(&G_display_lock);
 }

void IH_set_status_bar(char *app_name)
 {
   LCDdrawstring(0,1,app_name,TEXT_INVERTED);
   pthread_mutex_lock(&G_display_lock);
   LCDdisplay();
   pthread_mutex_unlock(&G_display_lock);
 }


scroll_list_item_t *IH_scroll_list_item_add(scroll_list_item_t *prev_item, char *item_name, uint8_t item_code)
 {

   scroll_list_item_t *new_item;

   new_item = malloc(sizeof(scroll_list_item_t));
   new_item->item_name = (char *)malloc(512);

   strncpy(new_item->item_name, item_name, 512);
   new_item->item_code = item_code;
   new_item->next_item = NULL;

   if(prev_item != NULL)
    {
     prev_item->next_item = new_item;
     new_item->prev_item = prev_item;
    }
   else
    new_item->prev_item = NULL;
   
   return new_item;

 }

char *IH_get_file_name_from_code(uint8_t code, scroll_list_item_t *first_item)
 {
  char *filename;
  scroll_list_item_t *curr_item;

  filename = malloc(1024);

  curr_item = first_item;

  while(curr_item != NULL)
   {
    if(curr_item->item_code == code)
     {
      if(strlen(curr_item->item_name) > 1024)
       {
        free(filename);
        return NULL;
       }
      strncpy(filename,curr_item->item_name, 1024);
      return filename;
     }
    curr_item = curr_item->next_item;
   }

  free(filename);
  return NULL;
 }

scroll_list_item_t *IH_get_file_list(char *directory)
 {
   uint8_t item_counter,first;
   DIR           *d;
   struct dirent *dir;
   scroll_list_item_t *first_item;
   scroll_list_item_t *next_item;
   scroll_list_item_t *prev_item;
   char *filename;

   first_item = NULL;
   item_counter = 1;
   first = 1;

   d = opendir(directory);

   if(d)
    {
      while ((dir = readdir(d)) != NULL)
      {
       if( ((dir->d_type == DT_REG) || (dir->d_type == DT_DIR)) && (strcmp(dir->d_name,".")!= 0) )
        {
         filename = NULL;
         if(dir->d_type == DT_REG)
          filename = strdup(dir->d_name);
         else if(dir->d_type == DT_DIR)
          {
           filename = malloc(strlen(dir->d_name)+2);
           filename[0] = '/';
           filename[1] = 0x0;
           strcat(filename,dir->d_name);
          }
         
         if(first)
          { 
           first_item = IH_scroll_list_item_add(NULL,filename,item_counter);
           prev_item = first_item;
           first = 0;
          }
         else
          {
           next_item = IH_scroll_list_item_add(prev_item,filename,item_counter);
           prev_item = next_item;
          }

        if(filename != NULL)
         free(filename);

        item_counter++;
       }
      }

    closedir(d);

    }
   else
    {
      return NULL;
    }

  return first_item;

 }

void IH_info(char *info_message)
 {
   uint8_t do_exit, key_event;

   do_exit = 0;

   LCDclear();
   IH_set_status_bar("Info message  ");
   IH_set_keymap_bar("","","","OK");

   LCDdrawstring(0,17,info_message,TEXT_NORMAL);
   pthread_mutex_lock(&G_display_lock);
   LCDdisplay();
   pthread_mutex_unlock(&G_display_lock);

   while(!do_exit)
    {
     read(G_keyboard_event_pipe[0],&key_event,1);
     switch(key_event)
       {
        case KEY4:
         return;
       }
    }

 }

uint8_t IH_yesno_dialog(char *dialog_message)
 {
   uint8_t do_exit, key_event;

   do_exit = 0;

   LCDclear();
   IH_set_status_bar(" Question     ");
   IH_set_keymap_bar("YES","   ","   ","NO ");

   LCDdrawstring(0,17,dialog_message,TEXT_NORMAL);
   pthread_mutex_lock(&G_display_lock);
   LCDdisplay();
   pthread_mutex_unlock(&G_display_lock);

   while(!do_exit)
    {
     read(G_keyboard_event_pipe[0],&key_event,1);
     switch(key_event)
       {
        case KEY1:
         return 1;
        case KEY4:
         return 0;
       }
    }

  return 0;

 }


void IH_quick_info(char *info_message)
 {
   // leave status bar and keymap bar as they are, just clear center of the display and show message 
   LCDdrawstring(0,11,"              ", TEXT_NORMAL);
   LCDdrawstring(0,21,"              ", TEXT_NORMAL);
   LCDdrawstring(0,17,info_message, TEXT_NORMAL);
   pthread_mutex_lock(&G_display_lock);
   LCDdisplay();
   pthread_mutex_unlock(&G_display_lock);

 }

uint8_t IH_edit_string(char **edit_string)
 {
   uint8_t do_exit,cursor_pos,cursor_pos_screen,key_event;

   do_exit = 0;
   cursor_pos = 0;
   cursor_pos_screen = 0;
   key_event = 0;
   char tmp_string[16];

   LCDclear();

   IH_set_status_bar("Enter name:");
   IH_set_keymap_bar("OK ","<- "," ->","ESC");
 
   memset(&tmp_string,32,16);
   tmp_string[15] = 0x0;

   while(!do_exit)
    { 
     LCDdrawstring(0,15,&tmp_string,TEXT_NORMAL);
     LCDdrawstring(0,25,"              ",TEXT_NORMAL);
     LCDdrawstring(cursor_pos_screen,25,"^",TEXT_NORMAL);
     pthread_mutex_lock(&G_display_lock);
     LCDdisplay();
     pthread_mutex_unlock(&G_display_lock);

     read(G_keyboard_event_pipe[0],&key_event,1);

     switch(key_event)
       {

        case ENC_KEY:
           if(cursor_pos < 13)
            {
             cursor_pos++;
             cursor_pos_screen += 6;
            }
           break;
           
        case ENC_UP:
           if(tmp_string[cursor_pos]<127)
             tmp_string[cursor_pos]++;
           break;

        case ENC_DOWN:
           if(tmp_string[cursor_pos]>32)
             tmp_string[cursor_pos]--;
           break;

        case KEY1:
           strncpy(*edit_string,tmp_string,16);
           return 1;
           break;

        case KEY2:
           if(cursor_pos > 0)
            {
             cursor_pos--;
             cursor_pos_screen -= 6;
            }
           break;

        case KEY3:
           if(cursor_pos < 13)
            {
             cursor_pos++;
             cursor_pos_screen += 6;
            }
           break;

        case KEY4: //ESC
           return 0;
       }
    }

  return 0;

 }

uint8_t IH_scroll_list(scroll_list_item_t *item_list_first_item, char *list_title)
 {

   scroll_list_item_t *curr_item, *next_item;
   char *display_string;

   uint8_t do_exit, key_event;
 
   display_string = malloc(16); 
   do_exit = 0;
   
   curr_item = item_list_first_item;

   LCDclear();

   IH_set_status_bar(list_title);
   IH_set_keymap_bar("OK ","   ","   ","ESC");

   while(!do_exit)
    {
     strncpy(display_string,curr_item->item_name,14);
     display_string[14] = 0x0;
     LCDdrawstring(0,12,"              ",TEXT_NORMAL);
     LCDdrawstring(0,12,display_string,TEXT_INVERTED);
     if(curr_item->next_item != NULL)
      {
        next_item = curr_item->next_item;
        strncpy(display_string,next_item->item_name,14);
        display_string[14] = 0x0;
        LCDdrawstring(0,21,"              ",TEXT_NORMAL);
        LCDdrawstring(0,21,display_string,TEXT_NORMAL);
      }
     else
      LCDdrawstring(0,21,"              ",TEXT_NORMAL);

     pthread_mutex_lock(&G_display_lock);
     LCDdisplay();
     pthread_mutex_unlock(&G_display_lock);

     read(G_keyboard_event_pipe[0],&key_event,1);

     switch(key_event)
      {
        case ENC_KEY:
            do_exit = 1;
            break; 
        case KEY1:
            do_exit = 1;
            break;
        case ENC_UP:
            if(curr_item->next_item != NULL)
             curr_item = curr_item->next_item;
            break;
        case ENC_DOWN:
            if(curr_item->prev_item != NULL)
             curr_item = curr_item->prev_item;
            break;
        case KEY4:
           free(display_string);
           return 0;
      }

    }

   free(display_string);
   return curr_item->item_code;

 }

void IH_scroll_list_destroy(scroll_list_item_t *first_item)
 {
  scroll_list_item_t *curr_item, *deleted_item;

  curr_item = first_item;

  while(curr_item->next_item != NULL)
   {
    deleted_item = curr_item;
    if(deleted_item == NULL)
     return;
    curr_item = deleted_item->next_item;
    free(deleted_item->item_name);
    free(deleted_item);
   }

 }

uint8_t IH_scroll_list_item_count(scroll_list_item_t *first_item)
 {

  scroll_list_item_t *curr_item;
  uint8_t item_counter;

  item_counter = 0;

  curr_item = first_item;

  while(curr_item->next_item != NULL)
   {
    if(curr_item == NULL)
     return 0;
    curr_item = curr_item->next_item;
    item_counter++;
   }

  return item_counter;

 }

