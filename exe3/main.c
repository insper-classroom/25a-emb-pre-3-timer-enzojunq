#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Pinos
const uint BTN_PIN_R = 28;
const uint LED_PIN_R = 4;

// Variáveis globais voláteis para sincronizar interrupções e loop principal
volatile bool btn_pressed = false;         // Indica que o botão está atualmente pressionado
volatile uint64_t press_start_time = 0;    // Armazena o tempo (ms) do momento da queda
volatile bool check_release = false;       // Indica que houve uma borda de subida e precisamos checar a duração

// Função de callback de interrupção do botão
void btn_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN_R) {
        // Borda de descida (botão pressionado)
        if (events & GPIO_IRQ_EDGE_FALL) {
            btn_pressed = true;
            // Armazena o tempo exato em que o botão foi pressionado
            press_start_time = to_ms_since_boot(get_absolute_time());
        }
        // Borda de subida (botão solto)
        else if (events & GPIO_IRQ_EDGE_RISE) {
            check_release = true;
        }
    }
}

int main() {
    stdio_init_all();

    // Configuração do LED
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0); // LED inicia apagado

    // Configuração do botão
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    // Habilita interrupções de borda de descida/subida
    gpio_set_irq_enabled_with_callback(
        BTN_PIN_R,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &btn_callback
    );

    // Loop principal
    while (true) {
        // Se detectamos que o botão foi solto, vamos checar quanto tempo ele ficou pressionado
        if (check_release) {
            check_release = false;  // Limpa a flag de borda de subida

            if (btn_pressed) {
                btn_pressed = false; // O botão não está mais pressionado
                uint64_t release_time = to_ms_since_boot(get_absolute_time());
                
                // Calcula a duração do aperto
                uint64_t press_duration = release_time - press_start_time;

                // Se o tempo de aperto foi >= 500 ms, consideramos "aperto longo"
                if (press_duration >= 500) {
                    // Toggla o LED
                    bool current_state = gpio_get(LED_PIN_R);
                    gpio_put(LED_PIN_R, !current_state);
                }
            }
        }

        // Pequeno atraso para não sobrecarregar a CPU (opcional)
        sleep_ms(1);
    }

    return 0; // Em teoria, nunca sai do loop
}