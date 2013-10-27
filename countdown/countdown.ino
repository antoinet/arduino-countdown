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
const int btn_pin_1 = 2;
const int btn_pin_2 = 3;
const int bzr_pin =  8;

// state variables
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
int state = STATE_WELCOME;
int btn_state_1 = 0;
int btn_state_2 = 0;
int counter = 0;
unsigned long timer = 0UL;

void setup() {
  // initialize pin modes
  pinMode(btn_pin_1, INPUT);
  pinMode(btn_pin_2, INPUT);
  pinMode(bzr_pin, OUTPUT);
  
  #ifdef DEBUG
  // enable serial debugging
  Serial.begin(9600);
  #endif
  
  // initialize matrix
  matrix.begin(0x70);
  matrix.setRotation(3);
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

inline void debug(const char *msg) {
  #ifdef DEBUG
    Serial.write(msg);
  #endif
}
  

void loop() {
  btn_state_1 = digitalRead(btn_pin_1);
  btn_state_2 = digitalRead(btn_pin_2);
  int color, i, j;
  
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
        state = STATE_COUNTDOWN;
      }
      break;
    
    case STATE_COUNTDOWN:
      debug("entering state countdown\n");
      if (millis() > (timer + DELAY)) {
        timer = millis();
        counter++;
      }
      
      if (btn_state_1 || btn_state_2) {
        debug("reset countdown\n");
        counter = 0;
        timer = millis();
        matrix.clear();
        matrix.writeDisplay();
        state = STATE_COUNTDOWN;
        return;
      }
      
      i = counter / 8;
      j = counter % 8;
      
      color = LED_GREEN;
      if (i > 3) {
        color = LED_YELLOW;
      }
      if (i > 6) {
        color = LED_RED;
      }
      matrix.drawPixel(j, i, color);
      matrix.writeDisplay();
      
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
