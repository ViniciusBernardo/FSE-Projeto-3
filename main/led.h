#ifndef LED_H
#define LED_H

#include "driver/gpio.h"
#define LED 2

struct led_state_machine {
    int on;
    int previous_state;
};

struct led_state_machine * initialize_led();
void control_led(struct led_state_machine * state_machine, int current_state);

#endif