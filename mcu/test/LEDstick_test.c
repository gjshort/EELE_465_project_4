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

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;

    // clear LED stick
    clearStick();

    while(1) {

        setColor(4, 10, 10, 10);
        sendStick();
        __delay_cycles(8000000);           // 1 sec delay          
    }

}
