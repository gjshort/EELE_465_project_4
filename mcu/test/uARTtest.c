#include <msp430fr2153.h>
#include "eUSCI.h"

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
// Setup eUSCI_A1
    init_eUSCI_A1();
    init_eUSCI_GPIO();

// Configure ports
    P3DIR |= BIT2;
    P3OUT &= ~BIT2;

	PM5CTL0 &= ~LOCKLPM5;

// Setup Timer
    TB0CTL |= TBCLR;
    TB0CTL |= TBSSEL__ACLK;    // Clear TimerB0, select 1Mhz clck, and use the UP counter
    TB0CTL |= MC__UP;

    TB0CCR0 = 32768;             // 1 second

// Setup Interrupts
//    TB0CCTL0 &= ~CCIFG;         // Clear CCR0 Flag
//    TB0CCTL0 |= CCIE;           // Enable CCR0 IRQ

    UCA1IE |= UCRXIE;
	__enable_interrupt();

	while(1) {}

	return 0;
}

/***************************
 *       IRQ
***************************/

// TimerB0 CCR0 will toggle the LED on
//#pragma vector = TIMER0_B0_VECTOR
//__interrupt void ISR_TB0_CCR0(void)
//{
//    P3OUT ^= BIT2;          // LED1 ON
//
//    position = 0;
//    UCA1IE |= UCTXCPTIE;
//    UCA1IFG &= ~UCTXCPTIFG;
//    UCA1TXBUF = s.WhatTimeIsIt[position];
//
//    TB0CCTL0 &= ~CCIFG;     // Clear CCR0 Flag
//}


#pragma vector = EUSCI_A1_VECTOR
__interrupt void ISR_EUSCI_A1(void) {

    if(UCA1RXBUF == 't') {
        CurrentTimePrompt();
        InputTime();
    }
    UCA1IFG &= ~UCTXCPTIFG;

}