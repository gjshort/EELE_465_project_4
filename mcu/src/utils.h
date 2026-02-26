#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include "eUSCI.h"
#include "rtc.h"

void pack_time_buffer(MCP7940N_time*, char*);
void uart_tx_time_data(char*, uint8_t);

/*
#include "led_bar.h"
#include "led_pattern.h"

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

void led_bar_pat_to_anodes(volatile LED_PATTERN*, volatile LED_BAR*);
void led_time_change_register(volatile LED_TIME*);
void led_time_to_anodes(volatile LED_BAR*, volatile LED_TIME*);

*/
#endif
