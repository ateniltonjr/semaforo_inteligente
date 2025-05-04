#ifndef BUTTONS_H
#define BUTTONS_H

#include "pico/stdlib.h"

// Definições de constantes
#define botaoA 5
#define botaoB 6
#define debounce_delay 300

void iniciar_botoes()
{
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
}

#endif