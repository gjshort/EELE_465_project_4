#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include "eUSCI.h"
#include "rtc.h"
#include "led_bar.h"
#include "led_pattern.h"

// UART MESSAGE IDs
#define ID_TIME 't'
#define ID_WINDOW 'w'
#define ID_TEMP 'c'
#define ID_ERR '!'

extern const uint16_t TB1_1_SEC;

extern const uint16_t TB2_5_MS;

extern const uint16_t TB0_1_SEC;
extern const uint16_t TB0_750_MS;
extern const uint16_t TB0_500_MS;
extern const uint16_t TB0_250_MS;

//-------------
// Temporary time struct
typedef struct LED_TIME
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t date;
    uint8_t month;
    uint8_t year;
    uint8_t reg_num;
} LED_TIME;

char parse_uart_msg(char*);
void parse_uart_time_msg(char*, MCP7940N_time*);
void parse_uart_window_msg(char*, uint8_t*);
void pack_time_buffer(MCP7940N_time*, char*);
void pack_temp_buffer(char*, char*);
void uart_tx_msg_data(char*, uint8_t);
void init_adc_a5();

uint8_t BCDtoDEC(uint8_t);
uint8_t DECtoBCD(uint8_t);

void led_bar_pat_to_anodes(volatile LED_PATTERN*, volatile LED_BAR*);
void led_time_change_register(volatile LED_TIME*);
void led_time_to_anodes(volatile LED_BAR*, volatile LED_TIME*);

#endif
