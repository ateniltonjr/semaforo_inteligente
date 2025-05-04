#include "tarefas_semaforo.h"

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