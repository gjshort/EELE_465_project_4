#include <msp430fr2153.h>
#include <stdint.h>
#include <stdbool.h>
#include "eUSCI.h"
#include "rtc.h"
#include "utils.h"

volatile I2C_MODE rtc_mode;

// Interrupt flags for main loop to service
volatile bool i2c_rx_irq;
volatile bool i2c_tx_irq;
volatile bool uart_txcpt_irq;
volatile bool write_to_rtc;
volatile bool read_from_rtc;

volatile bool uart_start_time_tx;
volatile uint8_t five_sec_counter;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    // Init globals
    i2c_rx_irq = false;
    i2c_tx_irq = false;
    uart_txcpt_irq = false;
    read_from_rtc = false;
    uart_start_time_tx = false;
    five_sec_counter = 0;

    // Init I2C peripheral
    init_eUSCI_B0_i2c();
    set_eUSCI_B0_slave_addr(MCP7940N_I2C_ADDR);
    // Set up to write initial time: 10:00:00, Thursday, Feb 19, 2026
    MCP7940N_time rtc_time = {0,0,0x10,0x05,0x20,0x05,0x26};
    uint8_t rtc_reg_ptr = 0x00;
    rtc_mode = I2C_WRITE;
    write_to_rtc = true;

    // Init UART peripheral
    init_eUSCI_A1_uart();
    uint8_t uart_time_data_ptr = 0x00;

    // ------------- TB1 ---------------
    // 1 sec.
    TB1CTL |= TBCLR;            // Clear timers and dividers
    TB1CTL |= TBSSEL__SMCLK;    // Use SMCLK (1 MHz)
    TB1CTL |= MC__UP;           // Use UP mode for compares

    TB1CTL |= CNTL_0;           // 16-bit counter
    TB1CTL |= ID__4;            // Divide by 4
    TB1EX0 |= TBIDEX__5;        // Divide by 5

    TB1CCR0 = 50000;            // 1 sec. - Heartbeat LED duty cycle

    //TB1CCTL0 &= ~CCIFG;       // Enable Interrupt
    //TB1CCTL0 |= CCIE;

    // Final Init
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    // -------------------- MAIN LOOP ---------------------------
    while(1)
    {
        // UART - Start Tx of time after an RTC read completed
        if(uart_start_time_tx == true)
        {
            // Load first byte of time data into Tx Buf
            uart_start_time_tx = false;   
            TB1CCTL0 &= ~CCIE;          // Don't let TB1 trigger during Tx

            UCA1IFG &= ~UCTXCPTIFG;
            UCA1IE |= UCTXCPTIE;
            uart_tx_time_data(&rtc_time, &uart_time_data_ptr);
            uart_time_data_ptr++;
        }
        

        //--------- IRQ Flag Checking ------------
        // I2C - Start a write
        if(write_to_rtc == true)
        {
            write_to_rtc = false;
            set_eUSCI_B0_count(RTC_NUM_TIME_REGS + 1);  // Write reg ptr + time registers
            UCB0CTLW0 |= UCTR;                          // Set B0 I2C to Tx mode
            UCB0CTLW0 |= UCTXSTT;                       // Start transmission
        }

        // I2C - Transmit register pointer preparing for read
        if(read_from_rtc == true)
        {
            read_from_rtc = false;
            set_eUSCI_B0_count(1);
            UCB0CTLW0 |= UCTR;
            UCB0CTLW0 |= UCTXSTT;
        }

        // I2C - Rx IRQ
        if(i2c_rx_irq == true)
        {
            i2c_rx_irq = false;
            rtc_read_time_reg(&rtc_time, &rtc_reg_ptr);
            if(rtc_reg_ptr == RTC_NUM_TIME_REGS - 1)    // Done reading
            {
                rtc_reg_ptr = 0;
                uart_start_time_tx = true;              // Start Tx of time to console
            }
            else                                        // Still reading
            {
                rtc_reg_ptr++;
            }
            
        }

        // I2C - Tx IRQ
        if(i2c_tx_irq == true)
        {
            i2c_tx_irq = false;
            if(rtc_mode == I2C_WRITE)                   // Writing time to RTC
            {
                rtc_write_time_reg(&rtc_time, &rtc_reg_ptr);
                if(rtc_reg_ptr == RTC_NUM_TIME_REGS)    // Done writing
                {
                    rtc_reg_ptr = 0;
                    // Enable TB1 IRQ after writing
                    TB1CCTL0 &= ~CCIFG;     
                    TB1CCTL0 |= CCIE;
                }
                else                                    // Still writing
                {
                    rtc_reg_ptr++;
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

        // UART - TXCPT IRQ
        if(uart_txcpt_irq == true)
        {
            uart_txcpt_irq = false;
            if(uart_time_data_ptr >= 22)
            {   // Reset ptr, disable interrupt
                uart_time_data_ptr = 0;
                UCA1IE &= ~UCTXCPTIE;
                TB1CCTL0 &= ~CCIFG;     
                TB1CCTL0 |= CCIE;
            }
            else 
            {   // Load next character. After, increment ptr
                uart_tx_time_data(&rtc_time, &uart_time_data_ptr);
                uart_time_data_ptr++;
            }
        }

        /*
        // UART - Rx IRQ
        if()
        {

        }
        */

    }
    // ----------------------- END MAIN ------------------------

}

//==============================================================

// Every five seconds, read from RTC
// (1 second timer)
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void) {

    if(five_sec_counter >= 5)
    {
        five_sec_counter = 0;
        read_from_rtc = true;
        rtc_mode = I2C_READ;
    }
    else 
    {
        five_sec_counter++;
    }
    
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

// UART ISR
#pragma vector = EUSCI_A1_VECTOR
__interrupt void EUSCI_A1_UART_ISR(void) {

    switch(UCA1IV)
    {
    case TXCPTIFG:              // Done transmitting byte
        UCA1IFG &= ~UCTXCPTIFG; // Clear IRQ flag
        uart_txcpt_irq = true;
        break;
    case RXIFG:                 // Receieved byte
        UCA1IFG &= ~UCRXIFG;    // Clear IRQ flag
        /*
        if(UCA1RXBUF == 't')
        {
            UCA1IFG &= ~UCTXCPTIFG;
            UCA1IE |= UCTXCPTIE;
            UCA1TXBUF = letter;
        }
        */
        break;
    default:
        break;
    }
    

}
