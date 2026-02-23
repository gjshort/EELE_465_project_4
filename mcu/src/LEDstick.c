#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "LEDstick.h"

RGB rgb[numPixels];

// Timing function for individual bit reading for each LED
void timeLED(uint8_t z) {

  if (z == 0) {                 // Low bit

    P3OUT |= BIT7;              // P3.7 HIGH
    __delay_cycles(3);          // ~0.4us at 8MHz (adjust for your clock)

    P3OUT &= ~BIT7;             // P3.7 LOW
    __delay_cycles(7);          // ~0.85us at 8MHz

  } else {                      // High bit

    P3OUT |= BIT7;              // P3.7 HIGH
    __delay_cycles(6);          // ~0.8us at 8MHz

    P3OUT &= ~BIT7;             // P3.7 LOW
    __delay_cycles(4);          // ~0.45us at 8MHz

  }
}


// Convert RGB values to binary, time the LED
void DotimeLED(uint8_t index) {

  uint8_t red   = rgb[index].red;
  uint8_t green = rgb[index].green;
  uint8_t blue  = rgb[index].blue;


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
void setPixelColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {

    rgb[index].red   = r;
    rgb[index].green = g;
    rgb[index].blue  = b;
    
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

// Init DCO to 8MHz
void init_8MHz() {

  __bis_SR_register(SCG0);                  // disable FLL, select ref clock
  CSCTL3 = SELREF__REFOCLK;

  // clear CSCTL0 register
  CSCTL0 = 0;
  CSCTL1 = DCORSEL_3;

  /* Set FLLN and FLLD for target frequency (8MHz / 32768Hz = 244.14)
     Formula: FDCO = (FLLN + 1) * (Fref / FLLD)
     8MHz = (243 + 1) * (32768Hz / 1) */
  CSCTL2 = FLLD_0 + 243;
  __delay_cycles(3);

  // Enable FLL
  __bic_SR_register(SCG0);
  while(CSCTL7 & (FLLUNLOCK | FLLUNLOCK1));

}
