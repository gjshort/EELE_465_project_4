#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "lmt87lpm.h"

volatile bool start_temp_adc;
volatile bool is_temp_adc_done;
volatile uint16_t temp_adc_val;

volatile uint16_t lmt87_temp;

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Init globals
    start_temp_adc = false;
    is_temp_adc_done = false;

    // Test LED
    P3SEL0 &= ~BIT2;    // Set to Digital IO
    P3SEL1 &= ~BIT2;

    P3DIR |= BIT2;      // Set as output
    P3OUT &= ~BIT2;     // Clear output

    // ------------- TB1 ---------------
    // 0.5 sec.
    TB1CTL |= TBCLR;            // Clear timers and dividers
    TB1CTL |= TBSSEL__SMCLK;    // Use SMCLK (1 MHz)
    TB1CTL |= MC__UP;           // Use UP mode for compares

    TB1CTL |= CNTL_0;           // 16-bit counter
    TB1CTL |= ID__4;            // Divide by 4
    TB1EX0 |= TBIDEX__5;        // Divide by 5

    TB1CCR0 = 25000;            // 0.5 sec. - ADC sample time

    TB1CCTL0 &= ~CCIFG;       // Enable Interrupt
    TB1CCTL0 |= CCIE;

    // ------------- Temp Sensor ADC --------------

    P1SEL1 |= BIT5;
    P1SEL0 |= BIT5;         // Set P1.5 to analog input
    
    //ADCCTL0
    ADCCTL0 &= ~ADCSHT;     //Clear sample and hold timer
    ADCCTL0 |= ADCSHT_2;    //16 ADCCLK cycles per sample
    ADCCTL0 |= ADCON;       //Turn ADC on

    //ADCCTL1
    ADCCTL1 |= ADCSHP;      //Sample signal comes from sample timer
    ADCCTL1 |= ADCSSEL_2;   //Use SMCLK (1 MHz)

    //ADCCTL2
    ADCCTL2 &= ~ADCRES;     //Clear resolution
    ADCCTL2 |= ADCRES_2;    //Set to 12 bit resolution
    
    // ADC IRQ
    ADCIFG &= ~ADCIFG0;     //Clear IRQ flag
    ADCIE |= ADCIE0;        //Enable conversion completion IRQ

    

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    while(true)
    {
        if(start_temp_adc)
        {
            start_temp_adc = false;
            ADCMCTL0 |= ADCINCH_5;      // Set ADC source to P1.5
            ADCCTL0 |= ADCENC | ADCSC;  // Start ADC conversion
            while((ADCIFG & ADCIFG0) == 0) {}   // Wait for conversion
        }

        if(is_temp_adc_done)
        {
            //lmt87_temp = (-0.0597)*temp_adc_val + 195;
            uint8_t i;
            // Run through LUT and if ADC val is >= to LUT value, pull its
            // associated temperature
            for(i = 0; i < sizeof(lmt87_temp_table[0])/sizeof(uint16_t); i++)
            {
                if(temp_adc_val >= lmt87_temp_table[1][i])
                {
                    lmt87_temp = lmt87_temp_table[0][i];
                    break;
                }
            }
            is_temp_adc_done = false;
        }

    }
}

// ADC conversion vector
#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR()
{
    is_temp_adc_done = true;
    temp_adc_val = ADCMEM0;
}

// 0.5 sec TB1 ISR
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void) 
{
    start_temp_adc = true;
    TB1CCTL0 &= ~CCIFG;
}
