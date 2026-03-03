#ifndef LED_BAR_H
#define LED_BAR_H

#include <stdbool.h>
#include <stdint.h>

// * P2.7, P2.6, P2.5, P2.4, P4.7, P4.6, P4.5, P4.4
//   used for 12 seg. LED bar anodes
// * P5.0, P5.1, P3.4 
//   used for 12 seg. LED bar cathodes

/**
 * Data structure to hold color and data
 * information about the 12 segment LED bar.
 */
typedef struct LED_BAR
{
    bool    red;            // Display color
    bool    green;
    uint8_t seg_data[3];    // Segment data
    uint8_t seg_ptr;        // Current active segment
} LED_BAR;

void init_led_bar_timer();
void init_led_bar();
void led_bar_clear_anodes();
void led_bar_write_anodes(volatile LED_BAR*);
void led_bar_set_cathode(volatile LED_BAR*);
void led_bar_update_anode_data(volatile LED_BAR*, uint8_t, uint8_t, uint8_t);
void led_bar_change_color(volatile LED_BAR*);

#endif
