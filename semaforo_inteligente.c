#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include "lib/matrixws.h"
#include "lib/display.h"
#include "lib/buzzer.h"
#include "pico/bootrom.h"

#define botaoA 5
#define botaoB 6
#define debounce_delay 300

// Variável atômica para controle de modo
volatile bool modo = false; // false = normal, true = noturno
volatile uint tempo_interrupcao = 0;

// Handles e flags de controle
TaskHandle_t task_normal = NULL;
TaskHandle_t task_beep = NULL;
TaskHandle_t task_noturno = NULL;
TaskHandle_t task_sonora = NULL;

// Mutex para controle de recursos compartilhados
SemaphoreHandle_t xDisplayMutex;
SemaphoreHandle_t xLedMutex;

void iniciar_botoes() {
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
}

void limpar_display() {
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void modo_normal(void *pvParameters) {
    while (true) {
        // Sinal verde
        cores_matriz(22, 0, BRILHO, 0);
        bf();
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Pode", 48, 30);
        ssd1306_draw_string(&ssd, "Atravessar", 24, 45);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(10000));

        // Sinal amarelo
        cores_matriz(22, BRILHO, BRILHO, 0);
        bf();
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Atencao", 32, 40);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Sinal vermelho
        cores_matriz(22, BRILHO, 0, 0);
        bf();
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Pare", 48, 40);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void beep_modo_normal(void *pvParameters) {
    while (true) {
        // Sinal verde
        buzzer_set_freq(buzzer, 500);
        vTaskDelay(pdMS_TO_TICKS(1000));
        buzzer_stop(buzzer);
        vTaskDelay(pdMS_TO_TICKS(9000));

        // Sinal amarelo
        for(int i = 0; i < 5; i++) {
            if(!modo) {
                buzzer_set_freq(buzzer, 200);
                vTaskDelay(pdMS_TO_TICKS(50));
                buzzer_stop(buzzer);
                vTaskDelay(pdMS_TO_TICKS(50));
            } else {
                break;
            }
        }
        if(modo) continue;
        
        vTaskDelay(pdMS_TO_TICKS(4500));

        // Sinal vermelho
        for(int j = 0; j < 5; j++) {
            if(!modo) {
                buzzer_set_freq(buzzer, 300);
                vTaskDelay(pdMS_TO_TICKS(500));
                buzzer_stop(buzzer);
                vTaskDelay(pdMS_TO_TICKS(1500));
            } else {
                break;
            }
        }
    }
}

void modo_noturno(void *pvParameters) {
    while (true) {
        cores_matriz(22, BRILHO, BRILHO, 0);
        bf();
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo noturno", 20, 15);
        ssd1306_draw_string(&ssd, "Atencao", 32, 35);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(500));

        desliga();
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo noturno", 20, 30);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

void sinalizacao_sonora(void *pvParameters) {
    while (true) {
        if(modo) {
            buzzer_set_freq(buzzer, 80);
            vTaskDelay(pdMS_TO_TICKS(100));
            buzzer_stop(buzzer);
            vTaskDelay(pdMS_TO_TICKS(1900));
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void finalizar_tarefas() {
    // Desativa todas as saídas primeiro
    desliga();
    buzzer_stop(buzzer);
    limpar_display();

    // Finaliza tarefas de forma segura
    if(task_normal != NULL) {
        vTaskDelete(task_normal);
        task_normal = NULL;
    }
    if(task_beep != NULL) {
        vTaskDelete(task_beep);
        task_beep = NULL;
    }
    if(task_noturno != NULL) {
        vTaskDelete(task_noturno);
        task_noturno = NULL;
    }
    if(task_sonora != NULL) {
        vTaskDelete(task_sonora);
        task_sonora = NULL;
    }
}

void alternar_modo() {
    // Garante que todas as tarefas sejam finalizadas antes de alternar
    finalizar_tarefas();
    
    modo = !modo;
    
    if (!modo) {
        xTaskCreate(modo_normal, "Modo Normal", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task_normal);
        xTaskCreate(beep_modo_normal, "Beep Normal", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task_beep);
        printf("Modo Normal\n");
    } else {
        xTaskCreate(modo_noturno, "Modo Noturno", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task_noturno);
        xTaskCreate(sinalizacao_sonora, "Sinal Sonoro", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task_sonora);
        printf("Modo noturno\n");
    }
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint tempo_atual = to_ms_since_boot(get_absolute_time());
    
    if(tempo_atual - tempo_interrupcao > debounce_delay) {
        if(gpio == botaoB) {
            reset_usb_boot(0, 0);
        }
        else if(gpio == botaoA) {
            alternar_modo();
        }
        tempo_interrupcao = tempo_atual;
    }
}

int main() {
    // Inicialização de hardware
    buzzer_init(buzzer, 10);
    sleep_us(10);
    buzzer_stop(buzzer);

    stdio_init_all();
    controle(PINO_MATRIZ);
    display_init();

    // Configuração dos botões
    iniciar_botoes();
    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicia no modo normal
    alternar_modo();
    
    // Inicia o scheduler
    vTaskStartScheduler();
    
    while(true) {
        tight_loop_contents();
    }
}