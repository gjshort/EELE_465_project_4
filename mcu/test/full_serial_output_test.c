#include <msp430fr2153.h>
#include <stdint.h>
#include <stdbool.h>
#include "eUSCI.h"
#include "rtc.h"
#include "utils.h"
#include "lmt87lpm.h"
#include "ring_buffer.h"
#include "my_float.h"

// IRQ flags
static volatile bool start_temp_adc;
static volatile bool is_temp_adc_done;

// Globals
static volatile uint16_t temp_adc_val;

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    
    // --------- Init globals ---------
    start_temp_adc = false;
    is_temp_adc_done = false;

    // --------- Init locals ----------
    // Temp. sensor
    uint16_t lmt87_temp;
    float lmt87_temp_avg;
    ring_buffer temp_buf = {{0}, 0};
    char lmt87_temp_str[8] = {0};

    // Init temperature sensor ADC 
    init_adc_a5();

    // ------------- TB1 ---------------
    // 0.5 sec.
    TB1CTL |= TBCLR;            // Clear timers and dividers
    TB1CTL |= TBSSEL__SMCLK;    // Use SMCLK (1 MHz)
    TB1CTL |= MC__UP;           // Use UP mode for compares

    TB1CTL |= CNTL_0;           // 16-bit counter
    TB1CTL |= ID__4;            // Divide by 4
    TB1EX0 |= TBIDEX__5;        // Divide by 5

    TB1CCR0 = 25000;            // 0.5 sec. - ADC sample time

    TB1CCTL0 &= ~CCIFG;         // Enable Interrupt
    TB1CCTL0 |= CCIE;

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    while(true)
    {
        // --------------- ADC --------------------
        // Start Temperature ADC conversion
         if(start_temp_adc)
        {
            start_temp_adc = false;
            ADCMCTL0 |= ADCINCH_5;              // Set ADC source to P1.5
            ADCCTL0 |= ADCENC | ADCSC;          // Start ADC conversion
            while((ADCIFG & ADCIFG0) == 0) {}   // Wait for conversion
        }
        // When ADC is done, take new temp reading and add to
        // ring buffer then average buffer
        if(is_temp_adc_done)
        {
            is_temp_adc_done = false;
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
            
            // Add new value to ring buf, re-average the buffer with
            // a specified window and then convert avg. to string
            ring_buf_push(&temp_buf, lmt87_temp);
            lmt87_temp_avg = ring_buf_average(&temp_buf, 5);

            // Clear string buffer then put new avg. temp into it in ASCII
            for(i = 0; i < sizeof(lmt87_temp_str); i++) 
            {
                lmt87_temp_str[i] = '\0';
            }
            ftoa_2(lmt87_temp_avg, lmt87_temp_str);
        }

    }

}

// ADC conversion ISR
#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR()
{
    is_temp_adc_done = true;
    temp_adc_val = ADCMEM0;     // Reading ADCMEM0 clears IRQ flag
}

// 0.5 sec TB1 ISR
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void) 
{
    start_temp_adc = true;      // Start temp. sensor ADC conversion
    TB1CCTL0 &= ~CCIFG;
}
