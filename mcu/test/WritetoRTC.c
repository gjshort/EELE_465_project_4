/******************************
* Author: Gabe Story
* DAte: 02.15.2026
* Class: EELE 465
* Purpose: This is test code referencing EELE 371 policy
* on writing the time to and RTC, RTC used is the MCP7940N
************************************************************/

#include <msp430fr2153.h>
#include <stdint.h>
#include "eUSCI.h"


/* Global Vars */
int buffrPntr = 0;

// Updated array with Oscillator Start and Battery Enable
const uint8_t WriteTime[] = {
    0x00, // Start Register Address
    0xAE, // Seconds: 46s + ST bit (0x80)
    0x23, // Minutes: 23m
    0x09, // Hours: 09h
    0x0B, // Day: Wednesday (3) + VBATEN (0x08)
    0x15, // Date: 15th
    0x02, // Month: Feb
    0x26  // Year: 26
};

/************
* main
******************/
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

/* Run initializations */

    // init eUSCI_B0 for I2C
    UCB0CTLW0 |= UCSWRST;

    UCB0CTLW0 |= UCSSEL__SMCLK;
    UCB0BRW = 10;

    UCB0CTLW0 |= UCMODE_3;
    UCB0CTLW0 |= UCMST;                 // I2C mode, setup in master mode
    UCB0I2CSA |= 0x6F;

    UCB0CTLW1 |= UCASTP_2;
    UCB0TBCNT |= sizeof(WriteTime);

    // Configure ports
    P1SEL1 &= ~BIT3;                    // SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;                    // SDA
    P1SEL0 |= BIT2;

    PM5CTL0 &= ~LOCKLPM5;

    UCB0CTLW0 &= ~UCSWRST;

/* Enable interrupts */

    UCB0IE |= UCTXIE0;
    __enable_interrupt();

/* Setup while loop */

    while(1) {

        
        UCB0CTLW0 |= UCTR;       // put int TX mode, continously send START   
        UCB0CTLW0 |= UCTXSTT;    // send Start
        __delay_cycles(2000);

    }
    return 0;
}

/******************
* Interrupt Service Routines
***************************************/

// This interrupt will keep count of WriteTime index pointer & will reset when we reach the last byte or increment if we haven't yet
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void) {

    if(buffrPntr == (sizeof(WriteTime) - 1)) {
        UCB0TXBUF = WriteTime[buffrPntr];
        buffrPntr = 0;
    } else {
        UCB0TXBUF = WriteTime[buffrPntr];
        buffrPntr++;
        __delay_cycles(2000);
    }

}
