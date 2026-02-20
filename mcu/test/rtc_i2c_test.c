#include <msp430fr2153.h>
#include <stdint.h>
#include <stdbool.h>
#include "eUSCI.h"
#include "rtc.h"

volatile I2C_MODE rtc_mode;
volatile uint8_t rtc_reg_ptr;

// Interrupt flags for main loop to service
volatile bool i2c_rx_irq;
volatile bool i2c_tx_irq;
volatile bool write_to_rtc;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    // Init I2C peripheral
    init_eUSCI_B0_i2c();
    set_eUSCI_B0_slave_addr(MCP7940N_I2C_ADDR);
    // Set up to write initial time: 10:00:00, Thursday, Feb 19, 2026
    MCP7940N_time rtc_time = {0,0,0x10,0x05,0x20,0x05,0x26};
    uint8_t rtc_reg_ptr = 0x00;
    rtc_mode = I2C_WRITE;
    write_to_rtc = true;

    // ------------- TB1 ---------------
    // 1 sec.
    TB1CTL |= TBCLR;            // Clear timers and dividers
    TB1CTL |= TBSSEL__SMCLK;    // Use SMCLK (1 MHz)
    TB1CTL |= MC__UP;           // Use UP mode for compares

    TB1CTL |= CNTL_0;           // 16-bit counter
    TB1CTL |= ID__4;            // Divide by 4
    TB1EX0 |= TBIDEX__5;        // Divide by 5

    TB1CCR0 = 50000;        // 1 sec. - Heartbeat LED duty cycle

    TB1CCTL0 &= ~CCIFG;         //Clear IRQ flag       
    TB1CCTL0 |= CCIE;           //Enable local interrupt

    // Final Init
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    // -------------------- MAIN LOOP ---------------------------
    while(1)
    {

        //--------- IRQ Flag Checking ------------
        if(write_to_rtc == true)
        {
            write_to_rtc = false;
            set_eUSCI_B0_count(RTC_NUM_TIME_REGS + 1);  // Write reg ptr + time registers
            UCB0CTLW0 |= UCTR;                          // Set B0 I2C to Tx mode
            UCB0CTLW0 |= UCTXSTT;                       // Start transmission
            while(!(UCB0IFG & UCTXIFG)) {}       
        }

        if(i2c_rx_irq == true)
        {
            i2c_rx_irq = false;
            // Do something with I2C Rx Buffer
            
        }

        if(i2c_tx_irq == true)
        {
            i2c_tx_irq = false;
            if(rtc_mode == I2C_WRITE)                   // Writing time to RTC
            {
                rtc_write_time_reg(&rtc_time, &rtc_reg_ptr);
                if(rtc_reg_ptr == RTC_NUM_TIME_REGS)    // Done writing
                {
                    rtc_reg_ptr = 0;
                }
                else                                    // Still writing
                {
                    rtc_reg_ptr++;
                }
            }
            else if(rtc_mode == I2C_READ)
            {
                UCB0TXBUF = RTC_SEC_REG;
            } 
        }

    }
    // ----------------------- END MAIN ------------------------

}

//==============================================================

// Every second, write to RTC
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void) {
    write_to_rtc = true;
    TB1CCTL0 &= ~CCIFG;
}

/**
 * I2C ISR
 */
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {

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
