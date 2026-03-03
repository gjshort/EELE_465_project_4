/***********************************
* Author:       Gabe Story
* Date:         02.26.2026
* Class:        EELE 456
* Purpose:      This h file will hold all the variables needed
                to operate the WS2812B LED stick
**********************************************************************/

#pragma once

// Number of LEDs in strip
#define PixNumber   10                  

// What the neopixels receive as a 1 or 0
#define highTide    0b11000000                  // (1)
#define lowTide     0b10000000                  // (0)

// Useful typedefs
typedef unsigned char u_char;
typedef unsigned int  u_int;

/* References:   
Citing for inspiration & useful how-to
that helped me get started.
               
                - Mjmeli: MSP430 | NeoPixel library https://github.com/mjmeli/MSP430-NeoPixel-WS2812-Library/tree/master
*/
