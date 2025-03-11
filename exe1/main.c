#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

const uint BTN_PIN_R = 28;
const uint LED_PIN_R = 4;

volatile bool button_pressed_flag = false;
volatile bool toggle_led_flag = false;

void btn_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN_R && (events & GPIO_IRQ_EDGE_FALL)) {
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

    repeating_timer_t blinkingTimer;
    bool timerRunning = false;

    while (true) {
        if (button_pressed_flag) {
            button_pressed_flag = false;
            if (!timerRunning) {
                add_repeating_timer_ms(
                    500,
                    blinking_timer_callback,
                    NULL,
                    &blinkingTimer
                );
                timerRunning = true;
            } else {
                cancel_repeating_timer(&blinkingTimer);
                timerRunning = false;
                gpio_put(LED_PIN_R, 0);
            }
        }
        if (toggle_led_flag) {
            toggle_led_flag = false;
            bool currentState = gpio_get(LED_PIN_R);
            gpio_put(LED_PIN_R, !currentState);
        }
        sleep_ms(1);
    }
    return 0;
}
