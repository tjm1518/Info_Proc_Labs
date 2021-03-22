#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
// This changes the speed of the thing
#define SLEEP_TIME 50000
FILE* fp;
alt_32 x_read;
alt_u16 x_val = 127<<8;
alt_up_accelerometer_spi_dev *  acc_dev;
alt_u8 send_counter = 0;
alt_u8 uintto7seg[10] = {0b00000010,0b11110011,0b00100101,0b00001101,0b10011001,0b01001001,0b01000010,0b00011111,0b000000001,0b00001001};




void update_hex0(alt_8 number){
IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, uintto7seg[number]);
IOWR(HEX1_BASE,0, uintto7seg[number]);
}



void led_write(alt_u8 led_pattern) {
    IOWR(LED_BASE, 0, led_pattern);
}

alt_16 no_overflow(alt_u16 last, alt_16 change) {
	if (change>0) return 0xFFFF-(alt_32)change<last?0xFFFF:last+change;
	if (change<0) return 0-change>last?0:last+change;
	return last;
}

void update_xval(){
    alt_up_accelerometer_spi_read_x_axis(acc_dev, & x_read);
    alt_8 multi = x_read>0?1:-1;
    if (x_read*multi < 8) x_val = no_overflow(x_val,8*multi);
    else if (x_read*multi < 16) x_val = no_overflow(x_val,32*multi);
    else if (x_read*multi < 32) x_val = no_overflow(x_val,96*multi);
    else if (x_read*multi < 64) x_val = no_overflow(x_val,256*multi);
    else if (x_read*multi < 128) x_val = no_overflow(x_val,384*multi);
    else if (x_read*multi < 192) x_val = no_overflow(x_val,768*multi);
    else x_val = no_overflow(x_val,1024*multi);
    led_write(x_val>>8);
    //printf("xvalue = %d, x_read %d\n",x_val,x_read);
    fprintf(fp,"%c\n",x_val>>8);
    usleep(SLEEP_TIME);
}



int main(){

  printf("Running..\n");

  int fd = open("/dev/jtag_uart",O_RDWR|O_NONBLOCK);
  printf("Opened File\n");

  fp = fdopen(fd, "r+");
  printf("Opened Stream\n");
  alt_u8 prompt;



  acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");

  x_val = 0;
  while(1) {
	  update_xval();
  	  prompt = getc(fp);
  	  if (prompt == 'a') {
  		  IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0xffffffff);
  		  prompt = 0;
  	  } else if (prompt == 'b') {
  		  IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0);
  		  prompt = 0;
  	  }

    }
    printf("Complete\n");

}