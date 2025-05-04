#ifndef LEDS_RGB_H
#define LEDS_RGB_H

#include "pico/stdlib.h"
#include <stdio.h>

#define red 13
#define blue 12
#define green 11

void iniciar_rgb()
{
    gpio_init(red);
    gpio_init(green);
    gpio_init(blue);

    gpio_set_dir(red, GPIO_OUT);
    gpio_set_dir(green, GPIO_OUT);
    gpio_set_dir(blue, GPIO_OUT);
}

void leds_rgb(bool r, bool g, bool b)
{
    gpio_put(red, r);
    gpio_put(green, g);
    gpio_put(blue, b);
}

#endif