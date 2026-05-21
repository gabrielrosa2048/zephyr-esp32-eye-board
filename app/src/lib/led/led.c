#include "led.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int led_init(void){

	if (!gpio_is_ready_dt(&led)) {

        return -1;
	}

    return gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
}

void led_off(void) {
 
    gpio_pin_set_dt(&led, 0);
}

void led_on(void) {
 
    gpio_pin_set_dt(&led, 1);
}

void led_toggle(void) {
 
    gpio_pin_toggle_dt(&led);
}