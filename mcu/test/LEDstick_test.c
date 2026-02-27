/***********************************
* Author:   Gabe Story
* Date:     02.26.2026
* Class:    EELE 456
* Purpose:  This c file will hold all the policy

            We will be able to set the color for individual
            pixels, and change brightness
**********************************************************************/

#include    <msp430fr2153.h>
#include    "LEDstick.h"

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Setup CLK, SPI
    init_CLK();
    init_SPI();
    init_LEDstick_color_button();

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;

    // clear LED stick
    clearStick();

    while(1) {

        if((P3IN & BIT5) != 0) {
            __delay_cycles(100);
            stickColor_change();
            __delay_cycles(8000000);
        }

        sendStick();
        //__delay_cycles(8000000);           // 1/2 sec delay          
    }

}
