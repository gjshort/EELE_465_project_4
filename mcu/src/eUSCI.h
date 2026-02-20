/************************************
* Author: Gabe Story
* Date: Friday, 02.13.2026
* Class:   EELE 465
* Purpose: header file to take control of uART protocol driver
*******************************************************************/
#ifndef eUSCI_H
#define eUSCI_H

#include <stdint.h>

#define RXIFG0 0x16
#define TXIFG0 0x18

typedef enum
{
    I2C_READ,
    I2C_WRITE
} I2C_MODE;

typedef struct {

    // char arrays needed for interfacing with the serial console
    char TimeSetSerial[8];
    char GetTimeSerial[19];
    char WhatTimeIsIt[26];

    char RTCdontWorkMsgOne[47];
    char RTCdontWorkMsgTwo[47];
    char RTCdontWorkMsgThree[53];
    char RTCdontWorkMsgFour[65];

    unsigned int position;

    int i;
    int j;
    int k;
    int l;

} Serial;

extern Serial s;

void init_eUSCI_A1();
void init_eUSCI_B0_i2c();
void set_eUSCI_B0_slave_addr(uint8_t);
void set_eUSCI_B0_count(uint8_t);
void init_eUSCI_GPIO();
void init_timerB1();
/*
void CurrentTimePrompt();
void MCP7940N_SetTime(MCP7940N_time*);
void MCP7940N_GetTime(MCP7940N_time*);
uint8_t BCDtoDEC(uint8_t);
uint8_t DECtoBCD(uint8_t);
void WriteRTC();
void TheTimeWeDontHave();
*/

#endif
