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

#define FLAKES_PER_CYCLE (3)
#define FLAKE_BOTTOM (500)
#define MAX_SNOW_AMNT (500)

#define SCREEN_WIDTH (640)
#define SCREEN_HEIGHT (480)
#define DRAW_SECTION_BOTTOM (400)

#define GPIO_TALKING 2
#define GPIO_SNOW_FALLING 3
#define GPIO_RAINDEER_ANIMATE 4
#define GPIO_LEFT_PUNCH 5
#define GPIO_RIGHT_PUNCH 6

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
    gpio_init(GPIO_TALKING);
    gpio_set_dir(GPIO_TALKING, GPIO_IN);

    gpio_init(GPIO_SNOW_FALLING);
    gpio_set_dir(GPIO_SNOW_FALLING, GPIO_IN);

    gpio_init(GPIO_RIGHT_PUNCH);
    gpio_set_dir(GPIO_RIGHT_PUNCH, GPIO_IN);

    gpio_init(GPIO_LEFT_PUNCH);
    gpio_set_dir(GPIO_LEFT_PUNCH, GPIO_IN);

    gpio_init(GPIO_RAINDEER_ANIMATE);
    gpio_set_dir(GPIO_RAINDEER_ANIMATE, GPIO_IN);

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

        if(gpio_get(GPIO_TALKING)) {
          mouth_counter++;
          mouth_counter %= 8;
        } else {
          mouth_counter = 0;
        }

        if(gpio_get(GPIO_SNOW_FALLING)) {
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

        if(gpio_get(GPIO_RAINDEER_ANIMATE)) {
          /*animate raindeer a single time until turned off and on again then can happen again*/
        }

        if(gpio_get(GPIO_LEFT_PUNCH)) {
          /*animate left punch a single time changes the santa drawing temp*/
        }

        if(gpio_get(GPIO_RIGHT_PUNCH)) {
          /*animate right punch a single time changes the santa drawing temp*/
        }

        fillRect(320, 440, SCREEN_WIDTH, 20, BLACK);
        write_score(score);
        frame_count++;
        sleep_ms(10);
   }

}
