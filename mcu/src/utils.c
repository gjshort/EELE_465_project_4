#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "rtc.h"
#include "eUSCI.h"

// UART MESSAGE IDs
#define TIME 't'
#define WINDOW 'w'

/**
 * Interprets the UART message ID to pass parsing off
 * @param buf - buffer full of incoming UART data
 * @param rtc_time - pointer to a create RTC struct 
 *
 * Format: [id, ,{data}]
 */
void parse_uart_msg(char *buf, MCP7940N_time *rtc_time)
{
    switch(buf[0])
    {
    case TIME: 
        parse_uart_time_msg(buf, rtc_time);
        break;
    case WINDOW:
        //parse_uart_window_msg();
        break;
    default:
        break;
    }
}

/**
 * Parses a message containing the time from UART
 * @param rtc_time - pointer to a defined struct for the RTC
 * @param buf - incoming UART message
 *
 * Format - [id, ,{data}] --> Data = HH:MM:SS MM/DD/YY\r\n\0
 */
void parse_uart_time_msg(char *buf, MCP7940N_time *rtc_time)
{
    char *token = strtok(buf + 2, ":");                 // Extract hours
    rtc_time->hours = DECtoBCD((uint8_t)atoi(token));

    token = strtok(NULL, ":");                          // Extract minutes
    rtc_time->minutes = DECtoBCD((uint8_t)atoi(token));

    token = strtok(NULL, " ");                          // Extract seconds
    rtc_time->seconds = DECtoBCD((uint8_t)atoi(token)) | ST_BIT;

    token = strtok(NULL, "/");                          // Extract month
    rtc_time->month = DECtoBCD((uint8_t)atoi(token));

    token = strtok(NULL, "/");                          // Extract day
    rtc_time->date = DECtoBCD((uint8_t)atoi(token));

    token = strtok(NULL, "\r\n\0");                          // Extract year
    rtc_time->year = DECtoBCD((uint8_t)atoi(token));
}

/**
 * Pack the given buffer with the RTC time in ASCII
 * @param rtc_time - pointer to a defined struct for the RTC
 * @param buf - buffer the time message will be written to.
 *              MUST BE OF LENGTH 20 OR MORE. USER BEWARE.
 *
 * Format:  HH:MM:SS MM/DD/YY\r\n\0
 *
 */
 void pack_time_buffer(MCP7940N_time *rtc_time, char *buf)
 {
    buf[0]  = ((rtc_time->hours & 0x30) >> 4) + '0';      // strip 12/24hr mode bit
    buf[1]  = (rtc_time->hours & 0x0F) + '0';
    buf[2]  = ':';
    buf[3]  = ((rtc_time->minutes & 0xF0) >> 4) + '0';
    buf[4]  = (rtc_time->minutes & 0x0F) + '0';
    buf[5]  = ':';
    buf[6]  = ((rtc_time->seconds & 0x70) >> 4) + '0';    // Strip ST bit
    buf[7]  = (rtc_time->seconds & 0x0F) + '0';
    buf[8]  = ' ';
    buf[9]  = ((rtc_time->month & 0x10) >> 4) + '0';      // Strip LPYR bit
    buf[10] = (rtc_time->month & 0x0F) + '0';
    buf[11] = '/';
    buf[12] = ((rtc_time->date & 0x30) >> 4) + '0';
    buf[13] = (rtc_time->date & 0x0F) + '0';
    buf[14] = '/';
    buf[15] = ((rtc_time->year & 0xF0) >> 4) + '0';
    buf[16] = (rtc_time->year & 0x0F) + '0';
    buf[17] = '\r';
    buf[18] = '\n';
    buf[19] = '\0';
 }

/**
 * Updates the A1 UART Tx Buffer with the next character 
 * in the buffer holding the formatted RTC time.
 * @param time_buf - buffer holding the formatted RTC time
 * @param time_data_idx - index of the next character to be written
 *
 * NO BOUNDS CHECKING. USER BEWARE
 *
 */
void uart_tx_time_data(char *time_buf, uint8_t time_data_idx)
{
    UCA1TXBUF = time_buf[time_data_idx];
}

/**
 * Converts an 8-bit BCD number to an 8-bit unsigned int
 * @param BCD - 8 bit value in BCD
 * @return 8-bit unsinged integer represntation
 */
uint8_t BCDtoDEC(uint8_t BCD) {
    return (uint8_t)( (BCD >> 4) * 10 + (BCD * 0x0F) );         // convert BCD to DEC
}

/**
 * Converts an 8-bit unsigned int to an 8-bit BCD number
 * @param DEC - 8 bit value in decimal
 * @return 8-bit BCD represntation
 */
uint8_t DECtoBCD(uint8_t DEC) {
    return (uint8_t)( (DEC / 10 << 4) | (DEC % 10) );           // convert DEC to BCD
}
