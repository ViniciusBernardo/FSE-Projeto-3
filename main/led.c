#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"

struct led_state_machine * initialize_led()
{
    struct led_state_machine * state_machine = (struct led_state_machine *)malloc(sizeof(struct led_state_machine));
    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    state_machine->on = 1;
    state_machine->previous_state = 1;
    return state_machine;
}

void control_led(struct led_state_machine * state_machine, int current_state)
{
    int previous_state = state_machine->previous_state;

    // current_state = 0 -> keep previous state
    // current_state = 1 -> keep LED on
    // current_state = 2 -> Blink
    // current_state = 3 -> Blink ONCE
    if((current_state == 1 || previous_state == 1) && (current_state != 2 && current_state != 3)){
        state_machine->on = 1;
        gpio_set_level(LED, state_machine->on);
        state_machine->previous_state = 1;
    } else if((current_state == 2 || previous_state == 2) && (current_state != 1 && current_state != 3)){
        state_machine->on = !state_machine->on;
        gpio_set_level(LED, state_machine->on);
        state_machine->previous_state = 2;
    } else if(current_state == 3){
        gpio_set_level(LED, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED, 1);
    }
}