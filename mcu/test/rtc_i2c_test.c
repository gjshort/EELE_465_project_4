#include <msp430fr2153.h>
#include <stdint.h>
#include <stdbool.h>
#include "eUSCI.h"
#include "rtc.h"

volatile I2C_MODE rtc_mode;
volatile bool i2c_rx_irq;
volatile bool i2c_tx_irq;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    // Init I2C peripheral
    init_eUSCI_B0_i2c();
    set_eUSCI_B0_slave_addr(MCP7940N_I2C_ADDR);
    // Set up to write initial time
    set_eUSCI_B0_count(9);
    rtc_mode = I2C_WRITE;

    // Final Init
    PM5CTL0 &= ~LOCKLPM5;
    __enable_interrupt();

    while(1)
    {
        if(i2c_rx_irq == true)
        {
            // Do something with I2C Rx Buffer
        }

        if(i2c_tx_irq == true)
        {
            // Do something with I2C Tx Buffer
        }

    }

}

//==============================================================

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
