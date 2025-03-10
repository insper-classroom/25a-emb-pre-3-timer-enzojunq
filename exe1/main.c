#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

const uint BTN_PIN_R = 28;
const uint LED_PIN_R = 4;

volatile bool button_pressed_flag = false;
volatile bool toggle_led_flag = false;

static repeating_timer_t blinking_timer;
bool timer_running = false;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        button_pressed_flag = true;
    }
}

bool blinking_timer_callback(repeating_timer_t *rt) {
    toggle_led_flag = true;
    return true;
}

int main() {
    stdio_init_all();
    
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_set_irq_enabled_with_callback(
        BTN_PIN_R,
        GPIO_IRQ_EDGE_FALL,
        true,
        &btn_callback
    );

    while (true) {
        if (button_pressed_flag) {
            button_pressed_flag = false;

            if (!timer_running) {
                add_repeating_timer_ms(
                    500,
                    blinking_timer_callback,
                    NULL,
                    &blinking_timer
                );
                timer_running = true;
            } else {
                cancel_repeating_timer(&blinking_timer);
                timer_running = false;
                gpio_put(LED_PIN_R, 0);
            }
        }

        if (toggle_led_flag) {
            toggle_led_flag = false;
            bool current_led_state = gpio_get(LED_PIN_R);
            gpio_put(LED_PIN_R, !current_led_state);
        }

        sleep_ms(1);
    }

    return 0;
}
