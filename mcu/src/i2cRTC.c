/**********************************************************************************************
/ Author:   Gabe Story
/ Date:     02.10-11.2026
/ Class:    EELE 465 -- Microcontroller App.
/ Project:  3 -- LED bar, uART, I2C/RTC
/ Purpose:  This is code for I2C/RTC functionality
*********************************************************************************************/

#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "eUSCI.h"

// Functions to setup and initialize I2C comms
void init_eUSCI_B0() {

    // eUSCI_B0 enters software reset, use smclk and enable master mode
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 |= UCSSEL__SMCLK;
    UCB0BRW = 10;

    UCB0CTLW0 |= UCMODE_3;
    UCB0CTLW0 |= UCMST;                 // I2C mode, setup in master mode
    UCB0I2CSA = MCP7940N_I2C_ADDR

    UCB0CTLW1 |= UCASTP_2;
    UCB0TBCNT = 0x07;                   // send over 1 byte of data, automatically stop
    
    UCB0CTLW0 &= ~UCSWRST;              // take eUSCI_B0 out of software reset

}

// Configure ports for I2C
void init_I2C_ports() {

    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT3;                    // P1.3 == SCL

    P1SEL0 |= BIT2;
    P1SEL1 &= ~BIT2;                    // P1.2 == SDA

}

// Funtions to convert between BCD & decimal
uint8_t BCDtoDEC(uint8_t BCD) {

    return (uint8_t)( (BCD >> 4) * 10 + (BCD * 0x0F) );         // convert BCD to DEC

}

uint8_t DECtoBCD(uint8_t DEC) {

    return (uint8_t)( (DEC / 10 << 4) | (DEC % 10) );           // convert DEC to BCD

}

// Set specified time to the RTC
void MCP7940N_SetTime(MCP7940N_time *time) {

    uint8_t WriteBuffr[8] = s.SetTimeSerial[8];          // register pointer + 7 data bytes to store init. time

    WriteBuffr[0] = RTC_SEC_REG;    // start at seconds register 0x00
    WriteBuffr[1] = DECtoBCD(time->seconds) | ST_BIT;           // start oscillator
    WriteBuffr[2] = DECtoBCD(time->minutes);
    WriteBuffr[3] = DECtoBCD(time->hours);                      // defaults to 24hr mode
    WriteBuffr[4] = DECtoBCD(time->weekday) | VBATEN_BIT;       // enable backup battery
    WriteBuffr[5] = DECtoBCD(time->date);
    WriteBuffr[6] = DECtoBCD(time->month);
    WriteBuffr[7] = DECtoBCD(time->year);

    // Overarching I2C master write conditions
//    UCB0I2CSA = MCP7940N_I2C_ADDR;  // set slave address
//    UCB0CTLW0 |= UCTR | UCTXSTT;    // place i2c module into transmitter mode, generate START condition

    int k = 0;
    UCB0TXBUF = WriteBuffr[k];
    if(k == sizeof(WriteBuffr) - 1) {
        WriteBuffr[k] = UCB0TXBUF;
        k = 0;
    } else {
        WriteBuffr[k] = UCB0TXBUF;
        k++;
    }


//        while((UCB0IFG & UCSTPIFG) == 0);          // wait for first message to send, then generate STOP
//            UCB0IFG &= ~UCSTPIFG;

}

// Receive time from RTC
void MCP7940N_GetTime(MCP7940N_time *time) {

    uint8_t ReadBuffr[7];           // register pointer + 7 data bytes to receive time

    // Set register pointer to seconds address, construct receive conditions
//    UCB0I2CSA = MCP7940N_I2C_ADDR;
//    UCB0CTLW0 |= UCTR | UCTXSTT;    // place i2c module into transmitter mode, generate START condition
//    while(!(UCB0IFG & UCTXIFG));
//    UCB0TXBUF + RTC_SEC_REG;        // point to seconds register
//    while(UCB0CTLW0 & UCTXSTT);     // make sure START is sent

    // Switch to reciever mode
//    UCB0CTLW0 &= ~UCTR;             // receive
//    UCB0CTLW0 |= UCTXSTT;           // send a restart
//    while(UCB0CTLW0 & UCTXSTT);     // wait for restart message to go through

    int l = 0;
    ReadBuffr[l] = UCB0RXBUF;
    if(l == sizeof(ReadBuffr) - 1) {
        UCB0RXBUF = ReadBuffr[l];
        l = 0;
    } else {
        UCB0RXBUF = ReadBuffr[1];
        l++;
    }

    // Convert BCD to DEC and store in the struct;
    // making sure to strip hardware bits from overwriting data
    time->seconds = BCDtoDEC(ReadBuffr[0] & 0x7F);  // strip ST bit
    time->minutes = BCDtoDEC(ReadBuffr[1]);
    time->hours   = BCDtoDEC(ReadBuffr[2] & 0x3F);  // strip 12/24hr mode bit
    time->weekday = BCDtoDEC(ReadBuffr[3] & 0x07);  // strip VBATEN bit
    time->date    = BCDtoDEC(ReadBuffr[4]);
    time->month   = BCDtoDEC(ReadBuffr[5] & 0x1F);  // strip leap year bit
    time->year    = BCDtoDEC(ReadBuffr[6]);

}
