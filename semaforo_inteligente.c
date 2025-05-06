#include "lib/tasks_criadas.h"

// Variáveis globais
volatile bool modo = false;      // Controla o modo de operação (false = normal, true = noturno)
volatile uint tempo_interrupcao = 0; // Armazena tempo da última interrupção para debounce

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