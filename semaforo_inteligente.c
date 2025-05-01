#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <stdio.h>
#include "lib/matrixws.h"
#include "lib/display.h"
#include "lib/buzzer.h"

void modo_normal()
{
    while (true)
    {
        // Sinal verde
        cores_matriz(22, 0, BRILHO, 0);
        bf();
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Sinal amarelo
        cores_matriz(22, BRILHO, BRILHO, 0);
        bf();
        vTaskDelay(pdMS_TO_TICKS(500));

        // Sinal vermelho
        cores_matriz(22, BRILHO, 0, 0);
        bf();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void modo_noturno()
{
    while (true)
    {
        cores_matriz(12, BRILHO, BRILHO, 0);
        bf();
        vTaskDelay(pdMS_TO_TICKS(500));

        desliga();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void sinalizacao_sonora()
{
    while (true)
    {
        buzzer_set_freq(buzzer, 80);
        vTaskDelay(pdMS_TO_TICKS(300));

        buzzer_stop(buzzer);
        vTaskDelay(pdMS_TO_TICKS(700));
    }
}

void vDisplay3Task()
{
    char str_y[5]; // Buffer para armazenar a string
    int contador = 0;
    
    while (true)
    {
        sprintf(str_y, "%d", contador); // Converte em string
        contador++;                     // Incrementa o contador
        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);           // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);  // Desenha uma string
        ssd1306_draw_string(&ssd, "  FreeRTOS", 10, 28); // Desenha uma string
        ssd1306_draw_string(&ssd, "Contador  LEDs", 10, 41);    // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 40, 52);          // Desenha uma string
        ssd1306_send_data(&ssd);                           // Atualiza o display
        sleep_ms(735);
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    buzzer_init(buzzer, 100);
    sleep_ms(100);
    buzzer_stop(buzzer);

    controle(PINO_MATRIZ);
    display_init();

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    stdio_init_all();

    xTaskCreate(modo_normal, "Blink Task Led1", configMINIMAL_STACK_SIZE,
         NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(modo_noturno, "Blink Task Led2", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(sinalizacao_sonora, "Cont Task Disp3", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}
