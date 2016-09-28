#include "common.h"
#include "global.h"

void SYS_init(void)
 {
    MIDI_init_MIDI_msg_lenghts();

    G_sysex_record_status = 0;
    G_received_sysex_msg_count = 0;
    G_saved_sysex_msg_count = 0;
    G_mididump_packet_chain = NULL;
    G_mididump_packet_count = 0;

    G_sequencer_PPQN = 192;
    G_sequencer_BPM = 120;

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

int8_t RPi_MIDI_init(void)
 {

    int8_t UART_fd;
    struct termios options;
    struct serial_struct serinfo;

    UART_fd = open("/dev/ttyAMA0", O_RDWR);

    if( UART_fd == -1)
     return -1;

    fcntl(UART_fd, F_SETFL, 0);
    tcgetattr(UART_fd, &options);
    cfsetispeed(&options, B38400);
    cfsetospeed(&options, B38400);
    cfmakeraw(&options);
    //options.c_cflag |= (CLOCAL | CREAD);
    //options.c_cflag &= ~CRTSCTS;
    if (tcsetattr(UART_fd, TCSANOW, &options) != 0)
     return -1;

    return UART_fd;

 }


int8_t MIDI_init(void)
 {
   return RPi_MIDI_init();
 }



