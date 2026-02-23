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
