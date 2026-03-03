#ifndef LED_PATTERN_H
#define LED_PATTERN_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Specifies an LED pattern number,
 * a pattern direction, and 16 bits
 * for the bits of the pattern.
 * To be used as the user wishes.
 */
typedef struct LED_PATTERN 
{
    uint8_t pattern_num;
    uint8_t pattern_dir;
    uint16_t pattern;
} LED_PATTERN;

void init_led_pattern_timer();
void led_pat_change_pattern_state(volatile LED_PATTERN*);
void led_pat_change_pattern(volatile LED_PATTERN*);

#endif
