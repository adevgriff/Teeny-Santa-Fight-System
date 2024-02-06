/**
 * Hunter Adams (vha3@cornell.edu)
 * 
 *
 * HARDWARE CONNECTIONS
   - GPIO 16 ---> VGA Hsync 
   - GPIO 17 ---> VGA Vsync 
   - GPIO 18 ---> VGA Green lo-bit --> 470 ohm resistor --> VGA_Green
   - GPIO 19 ---> VGA Green hi_bit --> 330 ohm resistor --> VGA_Green
   - GPIO 20 ---> 330 ohm resistor ---> VGA-Blue 
   - GPIO 21 ---> 330 ohm resistor ---> VGA-Red 
   - RP2040 GND ---> VGA-GND
 *
 * RESOURCES USED
 *  - PIO state machines 0, 1, and 2 on PIO instance 0
 *  - DMA channels obtained by claim mechanism
 *  - 153.6 kBytes of RAM (for pixel color data)
 *
 */

// VGA graphics library
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "vga16_graphics.h"

#include "real_santa.h"
#include "teenyat.h"

// Some globals for storing timer information
volatile unsigned int time_accum = 0;
unsigned int time_accum_old = 0 ;
char timetext[40];

void read_callback(teenyat *t, tny_uword addr, tny_word *data, uint16_t *delay) {

}

void write_callback(teenyat *t, tny_uword addr, tny_word data, uint16_t *delay) {

}

void system_log(teenyat *t, char *log_msg) {
  // Write some text
      setTextColor(4);
      setCursor(320, 460);
      setTextSize(1);
      writeString(log_msg);
}

// Timer interrupt
bool repeating_timer_callback(struct repeating_timer *t) {

    time_accum += 1 ;
    return true;
}

void errorReportingTrap(const char *msg) {
  
    while(true) {
      // Write some text
      setTextColor(WHITE) ;
      setCursor(65, 0) ;
      setTextSize(1) ;
      writeString(msg) ;

      // A brief nap
      sleep_ms(100);
    }
}

int main() {
    // Initialize stdio
    stdio_init_all();

    // Initialize the VGA screen
    initVGA() ;

    teenyat t;
    tny_init_from_unsigned_char_array(&t, , , read_callback, write_callback);
    tny_init_custom_log(&t, system_log);

    int mouth_counter = 0;

    drawImg(real_santa_img, 0, 0);


    drawPixel( 20, 20, 5);

    // Setup a 1Hz timer
    struct repeating_timer timer;
    add_repeating_timer_ms(-10, repeating_timer_callback, NULL, &timer);

    while(true) {

        drawImg(out_pimg, 0, 0);
        int height = counter * 2;

        fillRect(64 - 20, 128 - 32, 32, height, 0);

        // A brief nap
        if(time_accum % 10 == 0) {
          mouth_counter++;
          mouth_counter %= 8;
        }
   }

}
