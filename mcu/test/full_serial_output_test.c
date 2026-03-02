#include <msp430fr2153.h>
#include <stdint.h>
#include <stdbool.h>
#include "eUSCI.h"
#include "rtc.h"
#include "utils.h"
#include "lmt87lpm.h"
#include "ring_buffer.h"
#include "my_float.h"

// -------- IRQ flags ---------
// Temp sensor ADC
static volatile bool start_temp_adc;
static volatile bool is_temp_adc_done;

// I2C
static volatile bool i2c_rx_irq;
static volatile bool i2c_tx_irq;

// -------- Globals ----------
// I2C
static volatile I2C_MODE rtc_mode;
static volatile bool write_to_rtc;      // Start write transaction
static volatile bool read_from_rtc;     // Start read transaction

// Temp sensor ADC
static volatile uint16_t temp_adc_val;

int main(void)
{
    // Stop the watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    
    // --------- Init globals ---------
    start_temp_adc = false;
    is_temp_adc_done = false;
    i2c_rx_irq = false;
    i2c_tx_irq = false;
    read_from_rtc = false;

    // --------- Init locals ----------
    // Temp. sensor
    uint16_t lmt87_temp;
    ring_buffer temp_buf = {{0}, 0};    // Ring buffer of temps
    float lmt87_temp_avg;
    char lmt87_temp_str[8] = {0};       // Avg. temp in ASCII

    // I2C
    // Initial time: 10:00:00, Thursday, Feb 19, 2026
    MCP7940N_time rtc_time = {0,0,0x10,0x05,0x20,0x05,0x26};
    uint8_t rtc_reg_idx = 0x00;
    rtc_mode = I2C_WRITE;       // Write initial time to RTC
    write_to_rtc = true;

    // ------------- INIT MCU -----------------
    // Temperature sensor ADC 
    init_adc_a5();

    // I2C
    init_eUSCI_B0_i2c();
    set_eUSCI_B0_slave_addr(MCP7940N_I2C_ADDR);
    
    // TB1 
    TB1CTL |= TBCLR;
    TB1CTL |= TBSSEL__SMCLK;    // 1 MHz
    TB1CTL |= MC__UP;           // Up mode (compare)

    TB1CTL |= CNTL_0;           // 16-bit counter
    TB1CTL |= ID__4;            // Divide by 4
    TB1EX0 |= TBIDEX__5;        // Divide by 5

    TB1CCR0 = 25000;            // 0.5 sec.

    TB1CCTL0 &= ~CCIFG;         // Enable Interrupt
    TB1CCTL0 |= CCIE;

    // Disable low-power mode
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    while(true)
    {
        // --------------- I2C --------------------
        // Start a write
        if(write_to_rtc == true)
        {
            write_to_rtc = false;
            set_eUSCI_B0_count(RTC_NUM_TIME_REGS + 1);  // Write reg ptr + time registers
            UCB0CTLW0 |= UCTR;                          // Set B0 I2C to Tx mode
            UCB0CTLW0 |= UCTXSTT;                       // Start transmission
        }

        // Transmit register pointer preparing for read
        if(read_from_rtc == true)
        {
            read_from_rtc = false;
            set_eUSCI_B0_count(1);
            UCB0CTLW0 |= UCTR;
            UCB0CTLW0 |= UCTXSTT;
        }

        // Handle I2C Rx IRQ
        if(i2c_rx_irq == true)
        {
            i2c_rx_irq = false;
            rtc_read_time_reg(&rtc_time, &rtc_reg_idx);
            if(rtc_reg_idx == RTC_NUM_TIME_REGS - 1)    // Done reading
            {
                rtc_reg_idx = 0;
            }
            else                                        // Still reading
            {
                rtc_reg_idx++;
            }
            
        }

        // Handle I2C Tx IRQ
        if(i2c_tx_irq == true)
        {
            i2c_tx_irq = false;
            if(rtc_mode == I2C_WRITE)                   // Writing time to RTC
            {
                rtc_write_time_reg(&rtc_time, &rtc_reg_idx);
                if(rtc_reg_idx == RTC_NUM_TIME_REGS)    // Done writing
                {
                    rtc_reg_idx = 0;
                    // Enable TB1 IRQ after writing
                    TB1CCTL0 &= ~CCIFG;     
                    TB1CCTL0 |= CCIE;
                }
                else                                    // Still writing
                {
                    rtc_reg_idx++;
                }
            }
            else if(rtc_mode == I2C_READ)
            {
                // Load register ptr and wait to finish write
                UCB0TXBUF = RTC_SEC_REG;
                while(!(UCB0IFG & UCSTPIFG)) {}
                UCB0IFG &= ~UCSTPIFG;

                // Start read
                set_eUSCI_B0_count(RTC_NUM_TIME_REGS);
                UCB0CTLW0 &= ~UCTR;
                UCB0CTLW0 |= UCTXSTT;
            } 
        }

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
    TB1CCTL0 &= ~CCIFG;
    start_temp_adc = true;      // Start temp. sensor ADC conversion

    read_from_rtc = true;       // Read from RTC
    rtc_mode = I2C_READ;
}

// I2C ISR
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) 
{
    switch(UCB0IV) 
    {
    case RXIFG0:
        i2c_rx_irq = true;
        break;
    case TXIFG0:
        i2c_tx_irq = true;
        break;
    default:
        break;
    }
}
