#include <msp430fr2153.h>
#include "eUSCI.h"

volatile unsigned char SetTimeSerial[7];
volatile int rxIndex = -1;
int i;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
// Setup init
    init_eUSCI_A1();
    init_eUSCI_B0();
    init_eUSCI_GPIO();
    init_timerB1();

// Configure ports
    P3DIR |= BIT2;
    P3OUT &= ~BIT2;

	PM5CTL0 &= ~LOCKLPM5;


    TB1CCTL0 &= ~CCIFG;
    TB1CCTL0 |= CCIE;
    UCA1IE |= UCRXIE;
    //UCB0IE |= UCTXIE0;
    //UCB0IE |= UCRXIE0;
	__enable_interrupt();

	while(1) {

        // Setting up the Tx and Rx message conditions
        UCB0CTLW0 |= UCTR;
        UCB0TBCNT = 0x01;                          // put into Tx mode, send over 1 byte, start transmission
        UCB0CTLW0 |= UCTXSTT;

        __delay_cycles(1000);
            UCB0IFG &= ~UCSTPIFG;


        UCB0CTLW0 &= ~UCTR;
        UCB0TBCNT = 0x07;                          // put into Rx mode, receive over 7 bytes, start receiving
        UCB0CTLW0 |= UCTXSTT;

        __delay_cycles(1000);
            UCB0IFG &= ~UCSTPIFG;

	


        if(rxIndex == 7) {

            for(s.k=0; s.k<100; s.k++){}
            UCA1TXBUF = '\n';
            for(s.k=0; s.k<100; s.k++){}
            UCA1TXBUF = '\r';
            for(s.k=0; s.k<100; s.k++){}
	        UCA1TXBUF = '\0';

            TB1CCTL0 |= CCIE;

            // populate RTC time struct with received time from serial console
            MCP7940N_time newTime;
            newTime.seconds = SetTimeSerial[0];
            newTime.minutes = SetTimeSerial[1];
            newTime.hours   = SetTimeSerial[2];
            newTime.weekday = SetTimeSerial[3];
            newTime.date    = SetTimeSerial[4];
            newTime.month   = SetTimeSerial[5];
            newTime.year    = SetTimeSerial[6];

            MCP7940N_SetTime(&newTime);

            UCB0CTLW0 |= UCTR;
            UCB0CTLW0 |= UCTXSTT;               // START
            __delay_cycles(2000);


            P3OUT ^= BIT2;
            rxIndex = -1;
        }

    }

}

void InputTime(void) {
    rxIndex = 0;
}

/***************************
 *       IRQ
***************************/

// every 5 seconds you will see a prompt to give you the time via terminal
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void) {

    TheTimeWeDontHave();
    TB1CCTL0 &= ~CCIFG;

}

#pragma vector = EUSCI_A1_VECTOR
__interrupt void ISR_EUSCI_A1(void) {

    if(UCA1IFG & UCRXIFG) {

        if(rxIndex == -1) {
            if(UCA1RXBUF == 't') {
                TB1CCTL0 &= ~CCIE;
                CurrentTimePrompt();
                rxIndex = 0;
            } else if(UCA1RXBUF == 'g') {
                MCP7940N_GetTime();
            }
        } else {
            SetTimeSerial[rxIndex++] = UCA1RXBUF;
        }

    }

}

// This interrupt will keep count of packet index pointer & will reset when we reach the last byte or increment if we haven't yet
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {

    switch(UCB0IV) {
        case 0x16:
        WriteRTC();
    break;
        case 0x18:
        UCB0TXBUF = 0x00;
    break;
        default:
    break;

}

}
