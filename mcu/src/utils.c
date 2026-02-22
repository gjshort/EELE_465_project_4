#include <msp430fr2153.h>
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"
#include "rtc.h"
#include "eUSCI.h"

/**
 * Update the UART Tx Buffer with the necessary RTC register data.
 * Make sure to strip out the unnecessary register bits and insert
 * colons/commas where necessary.
 *
 */
void uart_tx_time_data(MCP7940N_time *rtc_time, uint8_t *time_data_ptr)
{
    switch(*time_data_ptr)
    {
    case 0: // Hours - Ten
        UCA1TXBUF = ((rtc_time->hours & 0x30) >> 4) + '0';      // strip 12/24hr mode bit
        break;
    case 1: // Hours - One
        UCA1TXBUF = (rtc_time->hours & 0x0F) + '0';
        break;
    case 2: // Colon
        UCA1TXBUF = ':';
        break;
    case 3: // Minutes - Ten
        UCA1TXBUF = ((rtc_time->minutes & 0xF0) >> 4) + '0';
        break; 
    case 4: // Minutes - One
        UCA1TXBUF = (rtc_time->minutes & 0x0F) + '0';
        break;  
    case 5: // Colon
        UCA1TXBUF = ':';
        break;
    case 6: // Seconds - Ten
        UCA1TXBUF = ((rtc_time->seconds & 0x70) >> 4) + '0';    // Strip ST bit
        break;
    case 7: // Seconds - One
        UCA1TXBUF = (rtc_time->seconds & 0x0F) + '0';
        break;
    case 8: // Space
        UCA1TXBUF = ' ';
        break;
    case 9: // Weekday
        UCA1TXBUF = (rtc_time->weekday & 0x07) + '0';           // Strip OSCRUN, PWRFAIL, VBAT bits
        break;
    case 10:    // Space
        UCA1TXBUF = ' ';
        break;
    case 11:    // Month - Ten
        UCA1TXBUF = ((rtc_time->month & 0x10) >> 4) + '0';      // Strip LPYR bit
        break;
    case 12:    // Month - One
        UCA1TXBUF = (rtc_time->month & 0x0F) + '0';
        break;
    case 13:    // Slash
        UCA1TXBUF = '/';
        break;  
    case 14:    // Date - Ten
        UCA1TXBUF = ((rtc_time->date & 0x30) >> 4) + '0';
        break;
    case 15:    // Date - One
        UCA1TXBUF = (rtc_time->date & 0x0F) + '0';
        break;
    case 16:    // Slash
        UCA1TXBUF = '/';
        break; 
    case 17:    // Year - Ten
        UCA1TXBUF = ((rtc_time->year & 0xF0) >> 4) + '0';
        break;
    case 18:    // Year - One
        UCA1TXBUF = (rtc_time->year & 0x0F) + '0';
        break;
    case 19:    // Carriage Return
        UCA1TXBUF = '\r';
        break;
    case 20:    // New line
        UCA1TXBUF = '\n';
        break; 
    case 21:    // Null
        UCA1TXBUF = '\0';
        break;  
    default:
        break;
    }
}

/*
#include "led_bar.h"
#include "led_pattern.h"

const uint16_t TB1_1_SEC  = 50000;

const uint16_t TB2_5_MS = 5000;

const uint16_t TB0_1_SEC  = 50000;
const uint16_t TB0_750_MS = 37500;
const uint16_t TB0_500_MS = 25000;
const uint16_t TB0_250_MS = 12500;
*/
/**
 * Writes an LED pattern to the anode data of an LED bar
 * @param led_bar_pattern - main's instance of an LED pattern struct
 * @param led_bar         - main's instance of an LED bar
 * 
 * Maps bits [11:8] to segment 0
 * Maps bits [7:4]  to segment 1
 * Maps bits [3:0]  to segment 2
 */
 /*
void led_bar_pat_to_anodes(volatile LED_PATTERN *led_bar_pattern, volatile LED_BAR *led_bar)
{
    led_bar->seg_data[0] = (uint8_t)(led_bar_pattern->pattern >> 8);
    led_bar->seg_data[1] = (uint8_t)(led_bar_pattern->pattern >> 4);
    led_bar->seg_data[2] = (uint8_t)(led_bar_pattern->pattern);
}
*/

/**
 * Advances the time register to be displayed
 * on the LED bar.
 * @param time - main's container of time data and reg. pointer
 */
 /*
void led_time_change_register(volatile LED_TIME *time)
{
    if(time->reg_num >= 5)
    {
        time->reg_num = 0;
    }
    else
    {
        time->reg_num++;
    }
}
*/

/**
 * Writes a time register to the LED bar anodes
 * @param led_bar - main's instance of the LED bar
 * @param time - main's container of time data and reg. pointer
 *
 * Takes the data in the field pointed to by reg_num and
 * packs it into the LED bar segment data.
 */
 /*
void led_time_to_anodes(volatile LED_BAR *led_bar, volatile LED_TIME *time)
{
    uint8_t seg1_data = 0, seg2_data = 0;
    // Pull proper field
    switch(time->reg_num)
    {
    case 0:
        seg1_data = time->seconds;
        break;
    case 1:
        seg1_data = time->minutes;
        break;
    case 2:
        seg1_data = time->hours;
        break;
    case 3:
        seg1_data = time->date;
        break;
    case 4:
        seg1_data = time->month;
        break;
    case 5:
        seg1_data = time->year;
        break;
    default:
        seg1_data = 0;
    }
    seg2_data = (seg1_data & 0x0F);         // Strip lower 4 bits
    seg1_data = (seg1_data & 0xF0) >> 4;    // Strip upper 4 bits

    led_bar_update_anode_data(led_bar, 0, seg1_data, seg2_data);
}
*/