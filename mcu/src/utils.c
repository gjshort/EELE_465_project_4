#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "rtc.h"
#include "eUSCI.h"

/**
 * Interprets the UART message ID
 * @param buf - buffer full of incoming UART data
 * @return - UART message ID
 * 
 * Format: [id, ,{data}]
 */
char parse_uart_msg(char *buf)
{
    switch(buf[0])
    {
    case ID_TIME: 
        return ID_TIME;
        break;
    case ID_WINDOW:
        return ID_WINDOW;
        break;
    default:
        return ID_ERR;
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

    token = strtok(NULL, "\r");                          // Extract year
    rtc_time->year = DECtoBCD((uint8_t)atoi(token));
}

void parse_uart_window_msg(char *buf, uint8_t *temp_avg_window)
{
    char *token = strtok(buf + 2, "\r");
    uint8_t window = (uint8_t)atoi(token);
    if(window > 16)
    {
        window = 16;
    }

    *temp_avg_window = window;
}

/**
 * Pack the given buffer with the RTC time in ASCII
 * @param rtc_time - pointer to a defined struct for the RTC
 * @param buf - buffer the time message will be written to.
 *              MUST BE OF LENGTH 22 OR MORE. USER BEWARE.
 *
 * Format: "d HH:MM:SS MM/DD/YY\r\n\0"
 *
 */
 void pack_time_buffer(MCP7940N_time *rtc_time, char *buf)
 {
    buf[0]  = ID_TIME;
    buf[1]  = ' ';
    buf[2]  = ((rtc_time->hours & 0x30) >> 4) + '0';      // strip 12/24hr mode bit
    buf[3]  = (rtc_time->hours & 0x0F) + '0';
    buf[4]  = ':';
    buf[5]  = ((rtc_time->minutes & 0xF0) >> 4) + '0';
    buf[6]  = (rtc_time->minutes & 0x0F) + '0';
    buf[7]  = ':';
    buf[8]  = ((rtc_time->seconds & 0x70) >> 4) + '0';    // Strip ST bit
    buf[9]  = (rtc_time->seconds & 0x0F) + '0';
    buf[10] = ' ';
    buf[11] = ((rtc_time->month & 0x10) >> 4) + '0';      // Strip LPYR bit
    buf[12] = (rtc_time->month & 0x0F) + '0';
    buf[13] = '/';
    buf[14] = ((rtc_time->date & 0x30) >> 4) + '0';
    buf[15] = (rtc_time->date & 0x0F) + '0';
    buf[16] = '/';
    buf[17] = ((rtc_time->year & 0xF0) >> 4) + '0';
    buf[18] = (rtc_time->year & 0x0F) + '0';
    buf[19] = '\r';
    buf[20] = '\n';
    buf[21] = '\0';
 }

/**
 * Pack the given buffer with the recent average temp. in ASCII
 * @param temp_str - an ASCII string representation of the temp.
 * @param buf - buffer the temp message will be written to.
 * 
 * Format: "c xxx.xxx\r\n\0" where xxx.xxx is a float in ASCII
 */
void pack_temp_buffer(char *temp_str, char *buf)
{
    buf[0] = ID_TEMP;
    buf[1] = ' ';

    // Copy over only necessary bytes from temp_str
    uint8_t i = 2;
    while(temp_str[i-2] != '\0')
    {
        buf[i] = temp_str[i-2];
        i++;
    }

    buf[i++] = '\r';
    buf[i++] = '\n';
    buf[i]   = '\0';
}

/**
 * Updates the A1 UART Tx Buffer with the next character 
 * in the buffer holding the formatted message
 * @param buf - buffer holding the formatted UART message
 * @param tx_msg_idx - index of the next character to be written
 *
 * NO BOUNDS CHECKING. USER BEWARE
 *
 */
void uart_tx_msg_data(char *buf, uint8_t tx_msg_idx)
{
    UCA1TXBUF = buf[tx_msg_idx];
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

/**
 * Inits the A5 ADC input
 */
void init_adc_a5()
{
    P1SEL1 |= BIT5;
    P1SEL0 |= BIT5;         // Set P1.5 to analog input
    
    //ADCCTL0
    ADCCTL0 &= ~ADCSHT;     //Clear sample and hold timer
    ADCCTL0 |= ADCSHT_2;    //16 ADCCLK cycles per sample
    ADCCTL0 |= ADCON;       //Turn ADC on

    //ADCCTL1
    ADCCTL1 |= ADCSHP;      //Sample signal comes from sample timer
    ADCCTL1 |= ADCSSEL_2;   //Use SMCLK (1 MHz)

    //ADCCTL2
    ADCCTL2 &= ~ADCRES;     //Clear resolution
    ADCCTL2 |= ADCRES_2;    //Set to 12 bit resolution
    
    // ADC IRQ
    ADCIFG &= ~ADCIFG0;     //Clear IRQ flag
    ADCIE |= ADCIE0;        //Enable conversion completion IRQ
}
