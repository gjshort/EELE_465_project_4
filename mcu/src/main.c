#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;

    while(true)
    {
    }
}
