// Inclusão de bibliotecas necessárias
#include "FreeRTOSConfig.h"   // Configurações do FreeRTOS
#include "hardware/gpio.h"   // Para controle de GPIO
#include "pico/bootrom.h"   // Funções do bootloader
#include "lib/leds_rgb.h"  // Controle dos LEDs RGB
#include "lib/matrixws.h" // Controle da matriz de LEDs
#include "lib/display.h" // Controle do display OLED
#include "lib/buttons.h"// Funções para botões
#include "lib/buzzer.h"// Controle do buzzer
#include "FreeRTOS.h" // Kernel do FreeRTOS
#include <stdio.h>   // Para funções de I/O
#include "task.h"   // Funções de tarefas

// Variáveis globais
volatile bool modo = false;      // Controla o modo de operação (false = normal, true = noturno)
volatile uint tempo_interrupcao = 0; // Armazena tempo da última interrupção para debounce

// Handles para as tarefas do FreeRTOS
TaskHandle_t task_normal = NULL;       // Tarefa do modo normal (LEDs)
TaskHandle_t task_beep_normal = NULL; // Tarefa do beep no modo normal
TaskHandle_t task_noturno = NULL;    // Tarefa do modo noturno
TaskHandle_t task_sonora = NULL;    // Tarefa de sinalização sonora

/*
    Tarefa do modo normal - controle dos LEDs e display
    Esta tarefa implementa o ciclo completo do semáforo:
    1. Verde (10 segundos)
    2. Amarelo (5 segundos)
    3. Vermelho (10 segundos)
 */
void modo_normal()
{  
    leds_rgb(0,0,0); //garante que os leds rgb desliguem antes
    while (true)
    {
        // Sinal verde - LED verde aceso
        cores_matriz(21, 0, BRILHO, 0);
        cores_matriz(22, 0, BRILHO, 0);
        cores_matriz(23, 0, BRILHO, 0);
        bf();
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Pode", 48, 30);
        ssd1306_draw_string(&ssd, "Atravessar", 24, 45);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(10000));

        // Sinal amarelo - LED amarelo aceso
        cores_matriz(21, BRILHO, BRILHO, 0);
        cores_matriz(22, BRILHO, BRILHO, 0);
        cores_matriz(23, BRILHO, BRILHO, 0);
        bf();
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Atencao", 32, 40);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Sinal vermelho - LED vermelho aceso
        cores_matriz(21, BRILHO, 0, 0);
        cores_matriz(22, BRILHO, 0, 0);
        cores_matriz(23, BRILHO, 0, 0);
        bf();
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Pare", 48, 40);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/*
    Tarefa de sinalização sonora no modo normal
    Implementa os sinais sonoros para cada estado do semáforo:
    - Verde: 1 beep longo (1s)
    - Amarelo: 5 beeps rápidos
    - Vermelho: 5 beeps médios
 */
void beep_modo_normal()
{
    while (true)
    {
        // Sinal verde - beep longo
        buzzer_set_freq(buzzer, 800);
        leds_rgb(0,1,0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        buzzer_stop(buzzer);
        leds_rgb(0,0,0);
        vTaskDelay(pdMS_TO_TICKS(9000));

        // Sinal amarelo - beeps rápidos
        for(int i = 0; i < 5; i++) 
        {   leds_rgb(1,1,0);
            buzzer_set_freq(buzzer, 500);
            vTaskDelay(pdMS_TO_TICKS(50));
            buzzer_stop(buzzer);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        leds_rgb(0,0,0);
        vTaskDelay(pdMS_TO_TICKS(4500));

        // Sinal vermelho - beeps médios
        for(int j = 0; j < 5; j++) 
        {
            buzzer_set_freq(buzzer, 600);
            leds_rgb(1,0,0);
            vTaskDelay(pdMS_TO_TICKS(500));
            buzzer_stop(buzzer);
            leds_rgb(0,0,0);
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
}

/*
    Tarefa do modo noturno
    Implementa o modo noturno com LED amarelo piscante:
    - 0.5s ligado
    - 1.5s desligado
 */
void modo_noturno() 
{
    while (true) 
    {
        // LED ligado (amarelo)
        cores_matriz(21, BRILHO, BRILHO, 0);
        cores_matriz(22, BRILHO, BRILHO, 0);
        cores_matriz(23, BRILHO, BRILHO, 0);
        bf();
        leds_rgb(1,1,0);
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo noturno", 20, 15);
        ssd1306_draw_string(&ssd, "Atencao", 32, 35);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(500));

        // LED desligado
        leds_rgb(0,0,0);
        limpar_display();
        ssd1306_draw_string(&ssd, "Modo noturno", 20, 15);
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

/*
    Tarefa de sinalização sonora noturna
    Implementa o sinal sonoro do modo noturno:
    - Beep curto a cada 2 segundos
 */
void sinalizacao_sonora() 
{
    while (true) 
    {
        buzzer_set_freq(buzzer, 1000);
        vTaskDelay(pdMS_TO_TICKS(100));
        buzzer_stop(buzzer);
        vTaskDelay(pdMS_TO_TICKS(1900));
    }
}

/* 
    Alterna entre os modos de operação
    Esta função:
        1. Finaliza todas as tarefas ativas
        2. Limpa display e LEDs
        3. Inicia as tarefas do novo modo
 */
void alternar_modo() 
{
    modo = !modo;
    
    // Finaliza todas as tarefas ativas
    if (task_normal != NULL) {
        vTaskDelete(task_normal);
        task_normal = NULL;
    }
    if (task_beep_normal != NULL) {
        vTaskDelete(task_beep_normal);
        task_beep_normal = NULL;
    }
    if (task_noturno != NULL) {
        vTaskDelete(task_noturno);
        task_noturno = NULL;
    }
    if (task_sonora != NULL) {
        vTaskDelete(task_sonora);
        task_sonora = NULL;
    }
    
    // Limpa hardware antes de iniciar novo modo
    limpar_display();
    desliga();
    buzzer_stop(buzzer);
    
    // Inicia tarefas do modo selecionado
    if (!modo) {
        xTaskCreate((TaskFunction_t)modo_normal, "Modo Normal", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task_normal);
        xTaskCreate((TaskFunction_t)beep_modo_normal, "Beep Normal", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task_beep_normal);
        printf("Modo Normal\n");
    } else {
        xTaskCreate((TaskFunction_t)modo_noturno, "Modo Noturno", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task_noturno);
        xTaskCreate((TaskFunction_t)sinalizacao_sonora, "Sinal Sonoro", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task_sonora);
        printf("Modo noturno\n");
    }
}

/*
    Manipulador de interrupção dos botões
    gpio GPIO que gerou a interrupção
    events Tipo de evento

    Implementa:
        - Debounce para evitar leituras múltiplas
        - Botão A: alterna modos
        - Botão B: entra em modo bootloader
 */
void gpio_irq_handler(uint gpio, uint32_t events) 
{
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

int main()
{   // Inicializa o buzzer com beep de 10us e 10Hz
    buzzer_init(buzzer, 10);
    sleep_us(10);
    buzzer_stop(buzzer);

    stdio_init_all();
    controle(PINO_MATRIZ);
    display_init();
    iniciar_rgb();

    iniciar_botoes();
    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    alternar_modo();
    
    vTaskStartScheduler();
    
    while(true) 
    {
        tight_loop_contents();
    }
}