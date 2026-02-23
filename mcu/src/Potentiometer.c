/************************************
* Author:   Gabe Story
* Date:     02.22.2026
* Class:    EELE 465
* Purpose:  Policy code for samppling ADC to control
            period of the LED patterns
****************************************************/

#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "Potentiometer.h"

ADCpot a;

// Initialize ADC to use A1
void init_ADCpot() {

    // P1.1 | pin 6 for A1
    P1SEL1 |= BIT1;
    P1SEL0 |= BIT1;

    // Configure ADC
    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |= ADCSHT_2;        // conversion cycles = 16
    ADCCTL0 |= ADCON;

    ADCCTL1 |= ADCSSEL_2;       // ADC use SMCLK
    ADCCTL1 |= ADCSHP;          
    
    ADCCTL2 &= ~ADCRES;         // 12-bit resolution
    ADCCTL2 |= ADCRES_2;

    ADCMCTL0 |= ADCINCH_1;      // ADC input channel to A1

}

// Timer setup for PWM
void init_PWMtimerB0() {

    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;    // Clear TimerB0, select ACLK, and use the UP counter
    TB0CTL |= MC__UP;

    TB0CCR0 = 32768;

}

void ADCpot_period() {

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
