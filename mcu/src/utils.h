#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include "eUSCI.h"
#include "rtc.h"

void parse_uart_msg(char*, MCP7940N_time*);
void parse_uart_time_msg(char*, MCP7940N_time*);
void pack_time_buffer(MCP7940N_time*, char*);
void uart_tx_time_data(char*, uint8_t);
void init_adc_a5();

uint8_t BCDtoDEC(uint8_t);
uint8_t DECtoBCD(uint8_t);

#endif
