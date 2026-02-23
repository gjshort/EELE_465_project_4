/***************************************
* Author:   Gabe Story 
* Date:     02.22.2026
* Class:    EELE 465
* Purpose:  Basic code to test ADC on P1.1 | A1 | pin 6
            If pin 6 receives certain voltage values the 
            period for toggling the LED bar changes
*************************************************************************************/

#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "Potentiometer.h"

ADCpot a;

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;

    // Init P3.2 for status LED
    P3DIR |= BIT2;
    P3OUT &= ~BIT2;

    // Init ADC for A1 | P1.1 | pin 6
    init_ADCpot();
    init_PWMtimerB0();

    // Enable interrupts
    TB0CCTL0 &= ~CCIFG;                     // clear CCR0 Flag
    TB0CCTL0 |= CCIE;                       // enable CCR0 IRQ
    __enable_interrupt();

    while(1) {

        ADCCTL0 |= ADCENC | ADCSC;          // enable & start conversion
        while((ADCIFG & ADCIFG0) == 0) {}
        a.ADCpot_value = ADCMEM0;

        /* This switch case satement reads voltage on pin 6
           and then changes the compare value, 
           chagning the period of the LED toggling */
        switch(a.ADCpot_value) {
            case 0:     TB0CCR0 = 32768;    // period of 1.0s
            break;

            case 256:   TB0CCR0 = 30933;    // period of 0.94s
            break;

            case 512:   TB0CCR0 = 29163;    // period of 0.89s
            break;

            case 768:   TB0CCR0 = 27197;    // period of 0.83s
            break;

            case 1024:  TB0CCR0 = 25559;    // period of 0.78s
            break;

            case 1280:  TB0CCR0 = 23593;    // period of 0.72s
            break;

            case 1536:  TB0CCR0 = 21627;    // period of 0.66s
            break;
            
            case 1792:  TB0CCR0 = 19988;    // period of 0.61s
            break;

            case 2048:  TB0CCR0 = 18022;    // period of 0.55s
            break;

            case 2304:  TB0CCR0 = 16384;    // period of 0.50s
            break;

            case 2560:  TB0CCR0 = 14418;    // period of 0.44s
            break;

            case 2816:  TB0CCR0 = 12452;    // period of 0.38s
            break;

            case 3072:  TB0CCR0 = 10813;    // period of 0.33s
            break;

            case 3328:  TB0CCR0 = 8847;     // period of 0.27s
            break;

            case 3584:  TB0CCR0 = 7209;     // period of 0.22s
            break;

            case 3840:  TB0CCR0 = 5243;     // period of 0.16s
            break;

            case 4096:  TB0CCR0 = 3277;     // period of 0.10s
            break;

            default:
            break;
        }

    }
    return 0;
}

// PWM interrupt
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void){

    
    TB0CCTL0 &= ~CCIFG;             // clear CCR0 Flag

}
