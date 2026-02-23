#ifndef GPIO_H
#define GPIO_H

// P3.2 = Heartbeat LED
// P3.3 = LED_Bar status LED
// P3.5 = LED_Bar mode change button
// P3.6 = LED_Bar state change button
// P3.7 = LED_Bar color change button
// P1.4 = LED_Bar reset button

void init_heartbeat_led();
void init_bar_status_led();
void init_bar_mode_button();
void init_bar_state_button();
void init_bar_color_button();
void init_bar_reset_button();
void init_gpio_timers();
void init_gpio();

#endif
