/************************************
* Author:   Gabe Story
* Date:     02.18.2026
* Class:    EELE 465
* Purpose:  Mechanism development for the LEDstick
*
*   Rec 8:  System can individually turn on each LED
*
*   Rec 9:  Color of LEDs can be changed by pressing a button
*        Spec 9.1:   St least need to be red, green, blue, & white
*
*   Rec 10: Position of slide potentiometer must be mapped
*            to lED strip
*
****************************************************************************/

#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "LEDstick.h"

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;

    // Set P3.7 for LEDstick I/O
    P3DIR |= BIT7;
    P3SEL0 &= ~BIT7;
    P3SEL1 &= ~BIT7;

    // Change MCLK to 8MHz
    CSCTL1 |= DCORSEL_3;
    CSCTL2 &= ~FLLD_1;
    CSCTL2 &= ~FLLN;
    CSCTL2 |= 244;

    // Init LED strip
    initLED();

    while(1) {

        setPixelColor(0, 55, 0, 0, 0);
        setPixelColor(1, 55, 55, 0, 0);
        setPixelColor(2, 0, 55, 0, 0);
        setPixelColor(3, 0, 55, 55, 0);
        setPixelColor(4, 0, 0, 55, 0);

        seeLED();

    }
    
    return 0;

}
