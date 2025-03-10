#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

const uint BTN_PIN_R = 28;
const uint BTN_PIN_G = 26;

const uint LED_PIN_R = 4;
const uint LED_PIN_G = 6;

volatile bool flag_r = false;
volatile bool flag_g = false;

volatile bool toggle_led_flag_r = false;
volatile bool toggle_led_flag_g = false;

static repeating_timer_t timer_r;
static repeating_timer_t timer_g;

bool timer_running_r = false;
bool timer_running_g = false;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_R) {
            flag_r = true;
        } else if (gpio == BTN_PIN_G) {
            flag_g = true;
        }
    }
}

bool blinking_timer_callback_r(repeating_timer_t *rt) {
    toggle_led_flag_r = true;
    return true;
}

bool blinking_timer_callback_g(repeating_timer_t *rt) {
    toggle_led_flag_g = true;
    return true;
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    gpio_put(LED_PIN_G, 0);

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        if (flag_r) {
            flag_r = false;
            if (!timer_running_r) {
                add_repeating_timer_ms(500, blinking_timer_callback_r, NULL, &timer_r);
                timer_running_r = true;
            } else {
                cancel_repeating_timer(&timer_r);
                timer_running_r = false;
                gpio_put(LED_PIN_R, 0);
            }
        }

        if (flag_g) {
            flag_g = false;
            if (!timer_running_g) {
                add_repeating_timer_ms(250, blinking_timer_callback_g, NULL, &timer_g);
                timer_running_g = true;
            } else {
                cancel_repeating_timer(&timer_g);
                timer_running_g = false;
                gpio_put(LED_PIN_G, 0);
            }
        }

        if (toggle_led_flag_r) {
            toggle_led_flag_r = false;
            bool current_r = gpio_get(LED_PIN_R);
            gpio_put(LED_PIN_R, !current_r);
        }

        if (toggle_led_flag_g) {
            toggle_led_flag_g = false;
            bool current_g = gpio_get(LED_PIN_G);
            gpio_put(LED_PIN_G, !current_g);
        }

        sleep_ms(1);
    }

    return 0;
}
