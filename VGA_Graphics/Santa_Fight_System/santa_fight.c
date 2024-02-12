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
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "vga16_graphics.h"

#include "real_santa.h"

#define FLAKES_PER_CYCLE (3)
#define FLAKE_BOTTOM (500)
#define MAX_SNOW_AMNT (500)

#define SCREEN_WIDTH (640)
#define SCREEN_HEIGHT (480)
#define DRAW_SECTION_BOTTOM (400)

#define GPIO_SDI0_TX (3)
#define GPIO_SDI0_RX (4)
#define GPIO_SDIO_SCK (2)
#define gPIO_SDIO_CS (5)

#define BUF_LEN         0x100

void printbuf(uint8_t buf[], size_t len) {
    int i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf(buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}

// Some globals for storing timer information
volatile unsigned int time_accum = 0;
unsigned int time_accum_old = 0 ;
char timetext[40];

typedef struct point {
  uint16_t x;
  uint16_t y;
} point;

void system_log(char *log_msg) {
  // Write some text
      setTextColor(4);
      setCursor(320, 460);
      setTextSize(1);
      writeString(log_msg);
}

void write_score(int16_t score) {
  char score_string[30];
  sprintf(score_string, "%d", score);
  setTextColor(3);
  setCursor(250, 440);
  setTextSize(1);
  writeString(score_string);
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
      sleep_ms(10);
    }
}

int main() {
    // Initialize stdio
    stdio_init_all();

    // Initialize the VGA screen
    initVGA() ;


    // Init GPIO
    printf("SPI slave example\n");

    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);
    spi_set_slave(spi_default, true);
    gpio_set_function(GPIO_SDI0_RX, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_SDIO_SCK, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_SDI0_TX, GPIO_FUNC_SPI);
    gpio_set_function(gPIO_SDIO_CS, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    bi_decl(bi_4pins_with_func(GPIO_SDI0_RX, GPIO_SDI0_TX, GPIO_SDIO_SCK, gPIO_SDIO_CS, GPIO_FUNC_SPI));

    uint8_t out_buf[BUF_LEN], in_buf[BUF_LEN];

    // Initialize output buffer
    for (size_t i = 0; i < BUF_LEN; ++i) {
        // bit-inverted from i. The values should be: {0xff, 0xfe, 0xfd...}
        out_buf[i] = ~i;
    }

    printf("SPI slave says: When reading from MOSI, the following buffer will be written to MISO:\n");
    printbuf(out_buf, BUF_LEN);

    int mouth_counter = 0;
    unsigned int frame_count = 0;

    // Setup a 1Hz timer
    struct repeating_timer timer;
    add_repeating_timer_ms(-250, repeating_timer_callback, NULL, &timer);

    uint8_t flake_index = 0;
    point snow[MAX_SNOW_AMNT];

    for(int i = 0; i < MAX_SNOW_AMNT; i++) {
      snow[i].x = 0;
      snow[i].y = FLAKE_BOTTOM;
    }

    int16_t score = 0;
    int16_t lives = 3;
    int16_t coal = 0;

    unsigned int snow_prev_time = time_accum;
    bool snow_fell = false;

    while(true) {

        drawImg(real_santa_img, 320 - 64, 200 - 64);
        int height = mouth_counter * 2;

        fillRect(320 - 20, 230, 32, height, 0);

        if(true) {
          mouth_counter++;
          mouth_counter %= 8;
        } else {
          mouth_counter = 0;
        }

        if(true) {
          /*create new snow at the top of the screen*/
          if(snow_prev_time - time_accum != 0) {
            snow_prev_time = time_accum;
            if(!snow_fell) {
              snow_fell = true;
              for(int i = 0; i < FLAKES_PER_CYCLE; i++) {
                uint16_t new_x = rand() % 640;
                snow[flake_index].x = new_x;
                snow[flake_index].y = 0;
                flake_index++;
                flake_index %= MAX_SNOW_AMNT;
              }
            }
          } else {
            snow_fell = false;
          }
        }

        
        /*update snow that is currently on the screen*/
        for(int i = 0; i < MAX_SNOW_AMNT; i++) {
          if(snow[i].y < FLAKE_BOTTOM) {
            drawPixel(snow[i].x, snow[i].y, BLACK);
            drawPixel(snow[i].x + 1, snow[i].y + 1, BLACK);
            drawPixel(snow[i].x, snow[i].y + 1, BLACK);
            drawPixel(snow[i].x + 1, snow[i].y, BLACK);
            snow[i].y++;
            /*x varies randomly by up to 3 pixels including temporarally off screen*/
            snow[i].x += ((rand() % 7) - 3);
            drawPixel(snow[i].x, snow[i].y, WHITE);
            drawPixel(snow[i].x + 1, snow[i].y + 1, WHITE);
            drawPixel(snow[i].x, snow[i].y + 1, WHITE);
            drawPixel(snow[i].x + 1, snow[i].y, WHITE);
          }
        } 

        if(true) {
          /*animate raindeer a single time until turned off and on again then can happen again*/
        }

        if(true) {
          /*animate left punch a single time changes the santa drawing temp*/
        }

        if(true) {
          /*animate right punch a single time changes the santa drawing temp*/
        }

        spi_write_read_blocking(spi0, out_buf, in_buf, BUF_LEN);
        // Write to stdio whatever came in on the MOSI line.
        printf("SPI slave says: read page %d from the MOSI line:\n", frame_count);
        printbuf(in_buf, BUF_LEN);

        fillRect(320, 440, SCREEN_WIDTH, 20, BLACK);
        write_score(score);
        frame_count++;
        sleep_ms(10);
   }

}
