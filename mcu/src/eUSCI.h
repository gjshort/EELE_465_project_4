/************************************
* Author: Gabe Story
* Date: Friday, 02.13.2026
* Class:   EELE 465
* Purpose: header file to take control of uART protocol driver
*******************************************************************/
#ifndef eUSCI_H
#define eUSCI_H

#include <stdint.h>

    // I2C address
    #define MCP7940N_I2C_ADDR       0x6F;

    // RTC Time Registers
    #define RTC_SEC_REG             0x00;               // seconds (also includes ST bit)
    #define RTC_MIN_REG             0x01;               // minutes
    #define RTC_HR_REG              0x02;               // hours
    #define RTC_WKDAY_REG           0x03;               // weekday (also inlcudes VBAT bit)
    #define RTC_DATE_REG            0x04;               // day of the month
    #define RTC_MTH_REG             0x05;               // month
    #define RTC_YR_REG              0x06;               // year
    #define CONTROL_REG             0x07;               // control / mode

    // ST and VBATEN configuration bits
    #define ST_BIT                  0x80;               // start oscillator bit (held in SEC register)
    #define VBATEN_BIT              0x08;               // enable backup battery (held in WKDAY register)

/***********
* struct to
* help manage the data 
*******************************/

typedef struct {

    uint8_t seconds;        // 0-59
    uint8_t minutes;        // 0-59
    uint8_t hours;          // 1-12 | 1-24
    uint8_t weekday;        // 1-7
    uint8_t date;           // 1-31
    uint8_t month;          // 1-12
    uint8_t year;           // 00-99
    char WriteBuffr[8];     // register pointer + 7 data bytes to store init. time
    char ReadBuffr[7];

} MCP7940N_time;

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
void init_eUSCI_B0();
void init_eUSCI_GPIO();
void init_timerB1();
void CurrentTimePrompt();
void MCP7940N_SetTime(MCP7940N_time*);
void MCP7940N_GetTime(MCP7940N_time*);
uint8_t BCDtoDEC(uint8_t);
uint8_t DECtoBCD(uint8_t);
void WriteRTC();
void TheTimeWeDontHave();

#endif
