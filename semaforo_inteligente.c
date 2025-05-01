// Inclusão de bibliotecas necessárias
#include "pico/stdlib.h"          // Biblioteca padrão do Pico
#include "hardware/gpio.h"        // Controle de GPIO
#include "FreeRTOS.h"            // Kernel do FreeRTOS
#include "FreeRTOSConfig.h"      // Configurações do FreeRTOS
#include "task.h"                // Funções de tarefas do FreeRTOS
#include <stdio.h>               // Funções de I/O padrão
#include "lib/matrixws.h"        // Biblioteca para controle da matriz de LEDs
#include "lib/display.h"         // Biblioteca para controle do display
#include "lib/buzzer.h"          // Biblioteca para controle do buzzer
#include "pico/bootrom.h"        // Funções do bootloader

// Definições de constantes
#define botaoA 5                 // Pino GPIO para o botão A
#define botaoB 6                 // Pino GPIO para o botão B
#define debounce_delay 300       // Tempo de debounce em ms

// Variáveis globais
volatile bool modo = true;      // Controla o modo de operação (false=normal, true=noturno)
volatile uint tempo_interrupcao = 0;  // Armazena tempo da última interrupção para debounce
TaskHandle_t task_normal = NULL; // Handle para tarefa do modo normal
TaskHandle_t task_noturno = NULL; // Handle para tarefa do modo noturno
TaskHandle_t task_sonora = NULL; // Handle para tarefa de sinalização sonora

// Função do modo normal de operação
void modo_normal()
{
    while (true)
    {
        // Sinal verde - LED verde aceso
        cores_matriz(22, 0, BRILHO, 0);
        bf();
        
        // Mensagem para sinal verde (centralizada)
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Pode", 48, 30);
        ssd1306_draw_string(&ssd, "Atravessar", 24, 45); 
        ssd1306_send_data(&ssd);
        ssd1306_fill(&ssd, false);
        
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Sinal amarelo
        cores_matriz(22, BRILHO, BRILHO, 0);
        bf();
        
        // Mensagem para sinal amarelo
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Atencao", 32, 40);
        ssd1306_send_data(&ssd);
        ssd1306_fill(&ssd, false);
        
        vTaskDelay(pdMS_TO_TICKS(500));

        // Sinal vermelho - LED vermelho aceso
        cores_matriz(22, BRILHO, 0, 0);
        bf();
        
        // Mensagem para sinal vermelho
        ssd1306_draw_string(&ssd, "Modo normal", 28, 15);
        ssd1306_draw_string(&ssd, "Pare", 48, 40);
        ssd1306_send_data(&ssd);
        ssd1306_fill(&ssd, false);
        
        vTaskDelay(pdMS_TO_TICKS(500));
        desliga();
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

// Função do modo noturno de operação
void modo_noturno()
{
    while (true)
    {
        cores_matriz(22, BRILHO, BRILHO, 0);
        bf();
        
        // Mensagem para modo noturno
        ssd1306_draw_string(&ssd, "Modo noturno", 20, 15); 
        ssd1306_draw_string(&ssd, "Atencao", 32, 35);
        ssd1306_send_data(&ssd);
        ssd1306_fill(&ssd, false);
        
        vTaskDelay(pdMS_TO_TICKS(500));

        desliga();
        
        ssd1306_draw_string(&ssd, "Modo noturno", 20, 15);
        ssd1306_send_data(&ssd);
        ssd1306_fill(&ssd, false);
        
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

// Função de sinalização sonora (usada no modo noturno)
void sinalizacao_sonora()
{
    while (true)
    {
        buzzer_set_freq(buzzer, 80); // Ativa buzzer com frequência 80Hz
        vTaskDelay(pdMS_TO_TICKS(100)); // Toca por 0.1 segundos

        buzzer_stop(buzzer); // Desliga o buzzer
        vTaskDelay(pdMS_TO_TICKS(1900)); // Pausa por 1.9 segundos
    }
}

// Função para configurar os botões
void iniciar_botoes()
{
    // Configura botão A como entrada com pull-up
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    // Configura botão B como entrada com pull-up
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
}

// Função para alternar entre modos de operação
void alternar_modo()
{
    modo = !modo; // Inverte o estado do modo
    
    // Remove as tarefas atuais se existirem
    if (task_normal != NULL) {
        vTaskDelete(task_normal);
        task_normal = NULL;
    }
    if (task_noturno != NULL) {
        vTaskDelete(task_noturno);
        task_noturno = NULL;
    }
    if (task_sonora != NULL) {
        vTaskDelete(task_sonora);
        task_sonora = NULL;
    }
    
    // Inicia o modo selecionado
    if (!modo) {
        // Modo normal: apenas a tarefa de LEDs
        xTaskCreate(modo_normal, "Modo Normal", configMINIMAL_STACK_SIZE,
                   NULL, tskIDLE_PRIORITY, &task_normal);
        printf("Modo Normal\n");
    } else {
        // Modo noturno: tarefa de LEDs piscantes e sinal sonoro
        printf("Modo noturno\n");
        xTaskCreate(modo_noturno, "Modo Noturno", configMINIMAL_STACK_SIZE, 
                   NULL, tskIDLE_PRIORITY, &task_noturno);
        xTaskCreate(sinalizacao_sonora, "Sinal Sonoro", configMINIMAL_STACK_SIZE, 
                   NULL, tskIDLE_PRIORITY, &task_sonora);
    }
}

// Manipulador de interrupção dos botões
void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint tempo_atual = to_ms_since_boot(get_absolute_time());
    
    // Verifica debounce
    if(tempo_atual - tempo_interrupcao > debounce_delay)
    {
        if(gpio == botaoB)
        {
            // Botão B: entra no modo bootloader USB
            reset_usb_boot(0, 0);
        }
        else if(gpio == botaoA)
        {
            // Botão A: alterna entre modos de operação
            alternar_modo();
        }
        
        tempo_interrupcao = tempo_atual; // Atualiza tempo da última interrupção
    }
}

// Função principal
int main()
{
    // Inicialização do buzzer (teste rápido)
    buzzer_init(buzzer, 100);
    sleep_ms(100);
    buzzer_stop(buzzer);

    // Inicializações
    stdio_init_all();           // Inicializa comunicação serial
    controle(PINO_MATRIZ);      // Configura controle da matriz de LEDs
    display_init();             // Inicializa o display

    // Configura botões e interrupções
    iniciar_botoes();
    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicia no modo normal por padrão
    alternar_modo();
    
    // Inicia o escalonador do FreeRTOS (duas chamadas iguais, provavelmente redundante)
    vTaskStartScheduler();
    vTaskStartScheduler();
    panic_unsupported(); // Nunca deve chegar aqui se o scheduler estiver rodando
    
    // Loop principal vazio (não deve ser alcançado)
    while(true)
    {
       tight_loop_contents(); // Otimização para loop vazio
    }
}