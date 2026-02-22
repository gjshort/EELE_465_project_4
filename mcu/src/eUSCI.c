/************************************
* Author: Gabe Story, Griffin Short
* Date: Saturday, 02.21.2026
* Class:   EELE 465
* Purpose: c file to take control of uART policy
*******************************************************************/

#include <msp430fr2153.h>
#include "eUSCI.h"

Serial s = {
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
    "The cur",
    "What is the current time? ",
    "The current time is: \r\n oh.... uhhh hold on...",
    "The current time is: \r\n huh..... nothing still",
    "How about you set the time? Press 't' to set the time",
    "Yeah... so this is chopped; \r\n Gabe couldn't get the RTC to work",
    0,
    0,
    0,
    0,
    0
};

/*
MCP7940N_time m = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    {0}
};
*/

void init_eUSCI_A1_uart() {

	UCA1CTLW0 |= UCSWRST;
	UCA1CTLW0 |= UCSSEL__SMCLK;         // 1 MHz reference clock
    // 115200 baud
	UCA1BRW = 8;
	UCA1MCTLW |= 0xD600;

    P4SEL1 &= ~BIT2;                    // Port 4.2 for uART Rx
    P4SEL0 |= BIT2;

	P4SEL1 &= ~BIT3;                    // Port 4.3 for uART Tx
	P4SEL0 |= BIT3;

	UCA1CTLW0 &= ~UCSWRST;

}

/**
 * Initialize the B0 eUSCI port for I2C
 * - Slave address set 
 */
void init_eUSCI_B0_i2c() {                  

    UCB0CTLW0 |= UCSWRST;

    UCB0CTLW0 |= UCSSEL__SMCLK;
    UCB0BRW = 10;                       // Divide by 10

    UCB0CTLW0 |= UCMODE_3;              // I2C Mode
    UCB0CTLW0 |= UCMST;                 // Master

    UCB0CTLW1 |= UCASTP_2;              // Auto stop

    P1SEL1 &= ~BIT3;                    // P1.3 for SCL
    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT2;                    // P1.2 for SDA
    P1SEL0 |= BIT2;

    UCB0CTLW0 &= ~UCSWRST;              // take eUSCI_B0 out of software reset

    UCB0IE |= UCTXIE0;                  //Enable wait for Tx msg IRQ
    UCB0IE |= UCRXIE0;                  //Enable Rx IRQ

}

/**
 * Sets the slave address for eUSCI B0
 */
void set_eUSCI_B0_slave_addr(uint8_t slave_addr)
{
    UCB0CTLW0 |= UCSWRST;
    UCB0I2CSA = slave_addr;             // I2C slave address
    UCB0CTLW0 &= ~UCSWRST;
    UCB0IE |= UCTXIE0;                  //Enable wait for Tx msg IRQ
    UCB0IE |= UCRXIE0;   
}

/**
 * Sets the count of bytes to be read or
 * received before generating an auto-stop
 */
void set_eUSCI_B0_count(uint8_t tbcnt)
{
    UCB0CTLW0 |= UCSWRST;
    UCB0TBCNT = tbcnt;
    UCB0CTLW0 &= ~UCSWRST;
    UCB0IE |= UCTXIE0;                  //Enable wait for Tx msg IRQ
    UCB0IE |= UCRXIE0;   
}

void init_timerB1() {

    TB1CTL |= TBCLR;
    TB1CTL |= TBSSEL__ACLK;    // Clear TimerB1, select 1Mhz clck, and use the UP counter
    TB1CTL |= MC__UP;
    TB1CTL |= ID__8;

    TB1CCR0 = 20479;             // 5 seconds

}

/*
// This function gets called when the user prsses 't' into the terminal, this function simply prompts "What's the current time? "
void CurrentTimePrompt() {

    P3OUT ^= BIT2;
	UCA1TXBUF = '\r';
    for(s.l=0; s.l<10000; s.l++){}

    for(s.position = 0; s.position < sizeof(s.WhatTimeIsIt); s.position++){
       UCA1TXBUF = s.WhatTimeIsIt[s.position];
       for(s.i=0; s.i<100; s.i=s.i+1) {}
   }
   	for(s.j=0; s.j<30000; s.j=s.j+1) {}

	for(s.k=0; s.k<100; s.k++){}
    UCA1TXBUF = '\n';
    for(s.k=0; s.k<100; s.k++){}
    UCA1TXBUF = '\r';
    for(s.k=0; s.k<100; s.k++){}
	UCA1TXBUF = '\0';

    s.l = 3;
}

// Set specified time to the RTC
void MCP7940N_SetTime(MCP7940N_time *time) {

    m.WriteBuffr[0] = RTC_SEC_REG;                                // start at seconds register 0x00
    m.WriteBuffr[1] = DECtoBCD(time->seconds) | ST_BIT;           // start oscillator
    m.WriteBuffr[2] = DECtoBCD(time->minutes);
    m.WriteBuffr[3] = DECtoBCD(time->hours);                      // defaults to 24hr mode
    m.WriteBuffr[4] = DECtoBCD(time->weekday) | VBATEN_BIT;       // enable backup battery
    m.WriteBuffr[5] = DECtoBCD(time->date);
    m.WriteBuffr[6] = DECtoBCD(time->month);
    m.WriteBuffr[7] = DECtoBCD(time->year);

}

// Receive time from RTC
void MCP7940N_GetTime(MCP7940N_time *time) {

    // Convert BCD to DEC and store in the struct;
    // making sure to strip hardware bits from overwriting data
    time->seconds = BCDtoDEC(m.ReadBuffr[0] & 0x7F);  // strip ST bit
    time->minutes = BCDtoDEC(m.ReadBuffr[1]);
    time->hours   = BCDtoDEC(m.ReadBuffr[2] & 0x3F);  // strip 12/24hr mode bit
    time->weekday = BCDtoDEC(m.ReadBuffr[3] & 0x07);  // strip VBATEN bit
    time->date    = BCDtoDEC(m.ReadBuffr[4]);
    time->month   = BCDtoDEC(m.ReadBuffr[5] & 0x1F);  // strip leap year bit
    time->year    = BCDtoDEC(m.ReadBuffr[6]);

    UCA1TXBUF = '\r';
    for(s.l = 0; s.l < 100; s.l = s.l+1) {}

    for(s.k = 0; s.k < sizeof(s.GetTimeSerial); s.k++) {
        UCA1TXBUF = s.GetTimeSerial[s.k];
        for(s.j = 0; s.j<100; s.j = s.j+1) {}
    }

	for(s.k=0; s.k<100; s.k++){}
    UCA1TXBUF = '\n';
    for(s.k=0; s.k<100; s.k++){}
    UCA1TXBUF = '\r';
    for(s.k=0; s.k<100; s.k++){}
	UCA1TXBUF = '\0';

    
    for(s.k = 0; s.k < sizeof(s.GetTimeSerial); s.k++) {
        UCA1TXBUF = m.ReadBuffr[s.k];
        for(s.j = 0; s.j<100; s.j = s.j+1) {}
    }

	for(s.k=0; s.k<100; s.k++){}
    UCA1TXBUF = '\n';
    for(s.k=0; s.k<100; s.k++){}
    UCA1TXBUF = '\r';
    for(s.k=0; s.k<100; s.k++){}
	UCA1TXBUF = '\0';

}

// Funtions to convert between BCD & decimal
uint8_t BCDtoDEC(uint8_t BCD) {

    return (uint8_t)( (BCD >> 4) * 10 + (BCD * 0x0F) );         // convert BCD to DEC

}

uint8_t DECtoBCD(uint8_t DEC) {

    return (uint8_t)( (DEC / 10 << 4) | (DEC % 10) );           // convert DEC to BCD

}

// Function called by eUSCI_B0 ISR to write to rtc
void WriteRTC() {

    if(s.i == (sizeof(m.WriteBuffr) - 1)) {
        UCB0TXBUF = m.WriteBuffr[s.i];
        s.i = 0;
    } else {
        UCB0TXBUF = m.WriteBuffr[s.i];
        s.i++;
    }

}

// Function reports the time every 5 seconds using TB1
void TheTimeWeDontHave() {

    // Go to start of line, new line

    switch(s.l) {
        case 0  :   for(s.position = 0; s.position < sizeof(s.RTCdontWorkMsgOne); s.position++){
                    UCA1TXBUF = s.RTCdontWorkMsgOne[s.position];
                    for(s.i=0; s.i<100; s.i=s.i+1) {}
                    }
   	                for(s.j=0; s.j<30000; s.j=s.j+1) {}

	                for(s.k=0; s.k<100; s.k++){}
                    UCA1TXBUF = '\n';
                    for(s.k=0; s.k<100; s.k++){}
                    UCA1TXBUF = '\r';
                    for(s.k=0; s.k<100; s.k++){}
	                UCA1TXBUF = '\0';

                    s.l++;
                    break;

        case 1  :   for(s.position = 0; s.position < sizeof(s.RTCdontWorkMsgTwo); s.position++){
                        UCA1TXBUF = s.RTCdontWorkMsgTwo[s.position];
                        for(s.i=0; s.i<100; s.i=s.i+1) {}
                    }
                    for(s.j=0; s.j<30000; s.j=s.j+1) {}

	                for(s.k=0; s.k<100; s.k++){}
                    UCA1TXBUF = '\n';
                    for(s.k=0; s.k<100; s.k++){}
                    UCA1TXBUF = '\r';
                    for(s.k=0; s.k<100; s.k++){}
	                UCA1TXBUF = '\0';

                    s.l++;
                    break;

        case 2  :   for(s.position = 0; s.position < sizeof(s.RTCdontWorkMsgThree); s.position++){
                        UCA1TXBUF = s.RTCdontWorkMsgThree[s.position];
                        for(s.i=0; s.i<100; s.i=s.i+1) {};
                    }
                    for(s.j=0; s.j<30000; s.j=s.j+1) {}

	                for(s.k=0; s.k<100; s.k++){}
                    UCA1TXBUF = '\n';
                    for(s.k=0; s.k<100; s.k++){}
                    UCA1TXBUF = '\r';
                    for(s.k=0; s.k<100; s.k++){}
	                UCA1TXBUF = '\0';
                    s.l = 0;
                    break;

        case 3  :   for(s.position = 0; s.position < sizeof(s.RTCdontWorkMsgFour); s.position++){
                        UCA1TXBUF = s.RTCdontWorkMsgFour[s.position];
                        for(s.i=0; s.i<100; s.i=s.i+1) {};
                    }
                    for(s.j=0; s.j<30000; s.j=s.j+1) {}

	                for(s.k=0; s.k<100; s.k++){}
                    UCA1TXBUF = '\n';
                    for(s.k=0; s.k<100; s.k++){}
                    UCA1TXBUF = '\r';
                    for(s.k=0; s.k<100; s.k++){}
	                UCA1TXBUF = '\0';
                    s.l = 0;
                    break;

        default :   break;
    }

    // Go to start of line, new line
    for(s.k=0; s.k<100; s.k++){}
    UCA1TXBUF = '\n';
    for(s.k=0; s.k<100; s.k++){}
    UCA1TXBUF = '\r';
    for(s.k=0; s.k<100; s.k++){}

}

*/
