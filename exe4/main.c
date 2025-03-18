#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;
const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueButId_r;
QueueHandle_t xQueueButId_g;


void gpio_callback(uint gpio, uint32_t events) {
    if (events == GPIO_IRQ_EDGE_FALL) {
        int valor = 100;               
        if (gpio == BTN_PIN_R) {
            xQueueSendFromISR(xQueueButId_r, &valor, 0);
        } else if (gpio == BTN_PIN_G) {
            xQueueSendFromISR(xQueueButId_g, &valor, 0);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0; // Delay inicial em ms
    int dummy;

    while (true) {
        // Aguarda até 500ms por um evento vindo da fila do botão vermelho
        if (xQueueReceive(xQueueButId_r, &dummy, pdMS_TO_TICKS(500)) == pdPASS) {
            if (delay < 1000)
                delay += 100;
            else
                delay = 100;
            printf("Red LED delay updated to: %d ms\n", delay);
        }
        if (delay > 0)
        {
            // Pisca o LED verde com o delay atual
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0; // Delay inicial em ms
    int dummy;

    while (true) {

        if (xQueueReceive(xQueueButId_g, &dummy, pdMS_TO_TICKS(500)) == pdPASS) {
            if (delay < 1000)
                delay += 100;
            else
                delay = 100;
            printf("Green LED delay updated to: %d ms\n", delay);
        }
        if (delay > 0) {

            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS\n");


    xQueueButId_r = xQueueCreate(32, sizeof(int));
    xQueueButId_g = xQueueCreate(32, sizeof(int));


    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);


    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);


    xTaskCreate(led_1_task, "LED_R_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_G_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
