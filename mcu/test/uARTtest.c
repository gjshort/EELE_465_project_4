#include <msp430fr2153.h>
#include <stdint.h>
#include <stdbool.h>
#include "eUSCI.h"

const char letter = '$';

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
// Setup eUSCI_A1
    init_eUSCI_A1_uart();

    // Setup 1 second interrupt timer
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__SMCLK;    // 1 MHz
    TB0CTL |= MC__UP;

    TB0CTL |= CNTL_0;           // 16-bit counter
    TB0CTL |= ID__4;            // Divide by 4
    TB0EX0 |= TBIDEX__5;        // Divide by 5

    TB0CCR0 = 50000;             // 1 second

    TB0CCTL0 &= ~CCIFG;         // Clear CCR0 Flag
    TB0CCTL0 |= CCIE;           // Enable CCR0 IRQ

    // Final Init
    PM5CTL0 &= ~LOCKLPM5;
	__enable_interrupt();

	while(1) {}

	return 0;
}

/***************************
 *       IRQ
***************************/

// TimerB0 CCR0 will toggle the LED on
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void)
{
    UCA1IFG &= ~UCTXCPTIFG;
    UCA1IE |= UCTXCPTIE;
    UCA1TXBUF = letter;
    
    TB0CCTL0 &= ~CCIFG;     // Clear CCR0 Flag
}


#pragma vector = EUSCI_A1_VECTOR
__interrupt void EUSCI_A1_UART_ISR(void) {

    switch(UCA1IV)
    {
    case TXCPTIFG:  // Done transmitting byte
    
        UCA1IE &= ~UCTXCPTIFG;  // Disable Interrupt
        UCA1IFG &= ~UCTXCPTIFG; // Clear IRQ flag
        break;
    case RXIFG:     // Receieved byte

        UCA1IFG &= ~UCRXIFG;    // Clear IRQ flag
        break;
    default:
        break;
    }
    

}
