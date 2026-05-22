#ifndef _LED_H_
#define _LED_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

int led_init(void);

void led_off(void);

void led_on(void);

void led_toggle(void);

#endif