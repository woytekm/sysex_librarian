#include "common.h"
#include "global.h"

void SYS_init(void)
 {
    MIDI_init_MIDI_msg_lenghts();

    G_log_to_curses = 1;
    G_sysex_record_status = 0;
    G_received_sysex_msg_count = 0;
    G_saved_sysex_msg_count = 0;

#ifdef IFACE_HW
    if(G_use_iface_hw == 1)
     {
      if(!bcm2835_init())
       {
        printf("fatal: cannot initialize BCM2835 library!");
        exit(-1);
       }  

      // one 74HC165 at these pins:
      bcm2835_gpio_fsel(GPIO17_PL_PIN11,BCM2835_GPIO_FSEL_OUTP);
      bcm2835_gpio_fsel(GPIO27_CP_PIN13,BCM2835_GPIO_FSEL_OUTP);
      bcm2835_gpio_fsel(GPIO22_CE_PIN15,BCM2835_GPIO_FSEL_OUTP);
      bcm2835_gpio_fsel(GPIO23_Q7_PIN16,BCM2835_GPIO_FSEL_INPT);
      
      LCDInit(60);

     }   
#endif

#ifdef IFACE_CURSES
    if(G_use_iface_curses == 1)
     SYS_init_curses_interface();
#endif

 }

void SYS_showhelp(void)
 {
   printf("command line options: \n");
   printf(" -c : use curses interface \n");
   printf(" -b : use interface board (hardware) \n");
   printf(" -h : show command line options \n");
 }

void SYS_debug(uint8_t debug_level, char *debug_msg, ...)
 {
  va_list ap;
  char message_cr[MAXMSG];

  if(debug_level <= DEBUG_LEVEL)
   {
    snprintf(message_cr,MAXMSG,"%s\n",debug_msg);
    va_start(ap, debug_msg);
    if(G_use_iface_curses)
     {
      vwprintw(G_log_window, message_cr, ap);
      wrefresh(G_log_window);
     }
    else
     {
      vfprintf(stderr, message_cr, ap);
     }
    va_end(ap);
   }
 }

void SYS_error(char *error_msg, ...)
 {

  va_list ap;
  char message_cr[MAXMSG];

  snprintf(message_cr,MAXMSG,"ERROR: %s\n",error_msg);
  va_start(ap, error_msg);
  vfprintf(stderr, message_cr, ap);
  va_end(ap);

 }


pthread_t SYS_start_task(uint8_t task_slot_id, void *task_function(void *params), void *task_args, int scheduling_policy, int priority)
 {

   uint8_t setstack_err=0;
   pthread_t new_task;
   pthread_attr_t new_task_attr;
   struct sched_param new_task_sched_param;

   pthread_attr_init(&new_task_attr);
   pthread_attr_setdetachstate(&new_task_attr, PTHREAD_CREATE_DETACHED);

   if( (setstack_err = pthread_attr_setstacksize(&new_task_attr, DEFAULT_THREAD_STACK_SIZE)) != 0)
    SYS_debug(DEBUG_NORMAL,"SYS_start_task: warning: cannot set thread stack size to %dk (%d)",
              DEFAULT_THREAD_STACK_SIZE,setstack_err);

   if(pipe(G_tasks[task_slot_id].input_pipe) == -1)
     SYS_debug(DEBUG_NORMAL,"SYS_start_task: warning: cannot create create input pipe for task %x",task_function);

   if(pthread_create(&new_task, &new_task_attr, task_function, task_args))
    {
     SYS_debug(DEBUG_NORMAL,"SYS_start_task: cannot start task %x",task_function);
     return -1;
    }

   new_task_sched_param.sched_priority = priority;

   if(pthread_setschedparam(new_task, scheduling_policy, &new_task_sched_param))
    SYS_debug(DEBUG_NORMAL,"SYS_start_task: warning: cannot set scheduling policy for task %x",task_function);

   G_task_count++;

   G_tasks[task_slot_id].task_id = new_task;

   return new_task;

 }


uint8_t SYS_write_sysex_buffer_to_disk(sysex_msg_t msg_array[], uint16_t msg_count, char *filename)
 {

   int fd;
   uint16_t cnt;
   uint32_t msg_len;
   
   if(chdir(DEFAULT_SYSEX_DIR) == -1)
    return 1;

   if( (fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC)) == -1 )
    return 1;

   for(cnt = 1; cnt <= msg_count; cnt++)
    {
      if( write(fd,(void *)msg_array[cnt].message, msg_array[cnt].length) == -1)
       {
         close(fd);
         return 1;
       }
    }

   close(fd);

   return 0;

 }

uint32_t SYS_read_sysex_buffer_from_file(char *filename, unsigned char **readin_buffer)
 {

   struct stat st;
   int fd;
   uint32_t readed, size;

   if(stat(filename, &st) == -1)
    {
     SYS_debug(DEBUG_NORMAL,"SYS: cannot stat file %s, errno: %d",filename,errno);
     return 0;
    }

   size = st.st_size;

   if(size == 0)
     return 0;

   *readin_buffer = (unsigned char *)malloc(size);

   fd = open(filename,O_RDONLY);

   readed = read(fd,*readin_buffer,size);

   SYS_debug(DEBUG_NORMAL,"SYS: readed %d bytes from input file",readed);

   if(size == readed)
    return size;
   else
    {
     free(*readin_buffer);
     return 0;
    }

   return 0;

 }

uint32_t SYS_free_sysex_buffer(void)
 {
   uint8_t msg_cnt;
   uint32_t total_bytes;

   total_bytes = 0;

   for(msg_cnt=1; msg_cnt <= G_saved_sysex_msg_count; msg_cnt++)
     {
       free(G_received_sysex_msgs[msg_cnt].message);
       total_bytes += G_received_sysex_msgs[msg_cnt].length;
       G_received_sysex_msgs[msg_cnt].length = 0;
     }

  G_saved_sysex_msg_count = 0;

  return total_bytes;

 }
