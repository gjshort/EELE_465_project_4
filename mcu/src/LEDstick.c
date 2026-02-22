#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "LEDstick.h"

RGB rgb[numPixels];

// Timing function for individual bit reading for each LED
void timeLED(uint8_t z) {

  if (z == 0) {                 // Low bit

    P3OUT |= BIT7;              // P3.7 HIGH
    __delay_cycles(3);          // ~0.25us at 8MHz (adjust for your clock)

    P3OUT &= ~BIT7;             // P3.7 LOW
    __delay_cycles(6);          // ~0.75us at 8MHz

  } else {                      // High bit

    P3OUT |= BIT7;              // P3.7 HIGH
    __delay_cycles(6);          // ~0.75us at 8MHz

    P3OUT &= ~BIT7;             // P3.7 LOW
    __delay_cycles(3);          // ~0.25us at 8MHz

  }
}


// Convert RGB values to binary, time the LED
void DotimeLED(uint8_t index) {

  uint8_t red   = rgb[index].red;
  uint8_t green = rgb[index].green;
  uint8_t blue  = rgb[index].blue;
  uint8_t white = rgb[index].white;

  uint8_t bit;
  for(bit = 8; bit > 0; --bit) {
    timeLED((green >> (bit - 1)) & 1);
  }
  for(bit = 8; bit > 0; --bit) {
    timeLED((red >> (bit - 1)) & 1);
  }
  for(bit = 8; bit > 0; --bit) {
    timeLED((blue >> (bit - 1)) & 1);
  }
  for(bit = 8; bit > 0; --bit) {
    timeLED((white >> (bit - 1)) & 1);
  }

}

// Send color to every LED
void seeLED() {

  uint8_t led;
  for(led = 0; led < 1; led++) {
    DotimeLED(led);
    __delay_cycles(400);
  }

}

// Helper function to change color
void setPixelColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {

    rgb[index].red   = r;
    rgb[index].green = g;
    rgb[index].blue  = b;
    rgb[index].white = w;
    
}

// Init LEDs to OFF
void initLED() {

  uint8_t i;
  for(i = 0; i < numPixels; i++) {
    rgb[i].red = rgb[i].green = rgb[i].blue = rgb[i].white = 0;
    for(i = 0; i < 24; i++) {
      timeLED(0);
    }
    __delay_cycles(400);
  }

}
