/*
 * Countdown
 *
 * A small countdown program for a custom shield comprising:
 *  * Adafruit 8x8 bicolor led matrix (http://www.adafruit.com/products/902)
 *  * A piezo buzzer
 *  * 2 push buttons
 *
 * Used libraries:
 *  * https://github.com/adafruit/Adafruit-LED-Backpack-Library
 *  * https://github.com/adafruit/Adafruit-GFX-Library
 *
 * Author:
 *  Antoine Neuenschwander <antoine@schoggi.org>
 * 
 */

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "constants.h"

//#define DEBUG

// pin assignments
const uint8_t btn_pin_1 = 2;
const uint8_t btn_pin_2 = 3;
const uint8_t bzr_pin =  8;

// state variables
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
uint8_t state = STATE_WELCOME;
uint8_t btn_state_1 = 0;
uint8_t btn_state_2 = 0;
uint8_t counter = 0;
boolean render_called = false;
unsigned long timer = 0UL;
uint8_t pixels[64];

void (*render[])(uint8_t) = {
  simple_row_traverser,
  random_dot_filler,
  spiralizer
};
void (*p_render)(uint8_t) = *render;

void setup() {
  // initialize pin modes
  pinMode(btn_pin_1, INPUT);
  pinMode(btn_pin_2, INPUT);
  pinMode(bzr_pin, OUTPUT);
  
  #ifdef DEBUG
  // enable serial debugging
  Serial.begin(9600);
  #endif
  
  // zero out pixels array
  memset(pixels, 0, sizeof(uint8_t)*64);
  
  // initialize matrix
  matrix.begin(0x70);
  matrix.setRotation(3);
  
  // init prng
  randomSeed(analogRead(0));
}


void loop() {
  // read button states
  btn_state_1 = digitalRead(btn_pin_1);
  btn_state_2 = digitalRead(btn_pin_2);
  
  switch (state) {
    case STATE_WELCOME:
      debug("entering state welcome\n");
      matrix.clear();
      matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_GREEN);
      matrix.writeDisplay();
      state = STATE_INIT;
      break;
    
    case STATE_INIT:
      debug("entering state init\n");
      if (btn_state_1 || btn_state_2) {
        counter = 0;
        timer = millis();
        matrix.clear();
        matrix.writeDisplay();
        render_called = false;
        p_render = render[random(sizeof(render)/sizeof(render[0]))];
        state = STATE_COUNTDOWN;
      }
      break;
    
    case STATE_COUNTDOWN:
      debug("entering state countdown\n");
      if (millis() > (timer + DELAY)) {
        timer = millis();
        render_called = false;
        counter++;
      }
      
      if (btn_state_1 || btn_state_2) {
        debug("reset countdown\n");
        state = STATE_INIT;
        return;
      }
      
      if (!render_called) {
        p_render(counter);
        render_called = true;
      }
      
      if (counter >= 64) {
        state = STATE_OVER;
      }
      break;
     
    case STATE_OVER:
       debug("entering state over\n");
       matrix.clear();
       matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_RED);
       matrix.blinkRate(1);
       matrix.writeDisplay();
       play_melody();
       matrix.blinkRate(0);
       state = STATE_INIT;
       break; 
  }
}

inline void debug(const char *msg) {
  #ifdef DEBUG
    Serial.write(msg);
  #endif
}

void play_melody() {
  int len = sizeof(melody)/sizeof(melody[0]);
  for (int i = 0; i < len; i++) {
    int duration = 1000/noteDurations[i];
    tone(bzr_pin, melody[i], duration);
    
    // 30% pause between notes
    int pause = duration * 1.30;
    delay(pause);
    
    noTone(bzr_pin);
  } 
}

void simple_row_traverser(uint8_t counter) {
  uint8_t color = LED_GREEN;
  uint8_t i = counter / 8;
  uint8_t j = counter % 8;
  
  if (i > 3) {
    color = LED_YELLOW;
  }
  if (i > 6) {
    color = LED_RED;
  }
  matrix.drawPixel(i, j, color);
  matrix.writeDisplay();
}

void random_dot_filler(uint8_t counter) {
  uint8_t color = LED_GREEN;
  
  if (!counter) {
    // generate a random permutation on the pixels array
    // see http://c-faq.com/lib/shuffle.html
    // and Knuth Sec. 3.4.2 pp. 137-8 
    for (uint8_t i = 0; i < 64; i++) {
      pixels[i] = i + 1;
    }
    for (uint8_t i = 0; i < 63; i++) {
      uint8_t c = random(64 - i);
      uint8_t t = pixels[i];
      pixels[i] = pixels[i+c];
      pixels[i+c] = t;
    }
  }
  
  if (counter >= 40) {
    color = LED_YELLOW;
  }
  if (counter >= 56) {
    color = LED_RED;
  }
  
  uint8_t i = pixels[counter] / 8;
  uint8_t j = pixels[counter] % 8;
  matrix.drawPixel(j, i, color);
  matrix.writeDisplay();
}

void spiralizer(uint8_t counter) {
  static uint8_t spiral[] = {
   0,  1,  2,  3,  4,  5,  6,  7,
   15, 23, 31, 39, 47, 55, 63,
   62, 61, 60, 59, 58, 57, 56,
   48, 40, 32, 24, 16, 8,
    9, 10, 11, 12, 13, 14,
   22, 30, 38, 46, 54,
   53, 52, 51, 50, 49,
   41, 33, 25, 17,
   18, 19, 20, 21,
   29, 37, 45,
   44, 43, 42,
   34, 26,
   27, 28,
   36, 35
  };
  
  int color = LED_GREEN;
  if (counter >= 40) {
    color = LED_YELLOW;
  }
  if (counter >= 56) {
    color = LED_RED;
  }
  
  uint8_t i = spiral[counter] / 8;
  uint8_t j = spiral[counter] % 8;
  
  matrix.drawPixel(j, i, color);
  matrix.writeDisplay();
}
