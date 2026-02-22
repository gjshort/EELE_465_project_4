/************************************
* Author:   Gabe Story
* Date:     02.22.2026
* Class:    EELE 465
* Purpose:  Policy code for samppling ADC to control
*           period of the LED patterns
****************************************************/

#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "Potentiometer.h"

// Initialize ADC to use A8
void init_ADCpot() {

    // P5.0 | pin 37 for A8
    P5SEL1 |= BIT0;
    P5SEL0 |= BIT0;

    // Configure ADC
    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |= ADCSHT_2;        // conversion cycles = 16
    ADCCTL0 |= ADCON;

    ADCCTL1 |= ADCSSEL_2;       // ADC use SMCLK
    ADCCTL1 |= ADCSHP;          
    
    ADCCTL2 &= ~ADCRES;         // 12-bit resolution
    ADCCTL2 |= ADCRES_2;

    ADCMCTL0 |= ADCINCH_8;      // ADC input channel to A8

}

