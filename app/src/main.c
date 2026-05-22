#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>

#include "led.h"

static void on_key_event(struct input_event *evt, void *user_data);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(adc_keys)), on_key_event, NULL);

int main(void){

	LOG_INF("ESP32-S3-EYE started");
    
    led_init();

    while(1){

		k_sleep(K_SECONDS(1));
	}

	return 0;
}

static void on_key_event(struct input_event *evt, void *user_data){

    if (evt->type != INPUT_EV_KEY || evt->value == 0) {
        
        return; 
    }

    switch (evt->code) {
        
        case INPUT_KEY_MENU: 
            LOG_INF("MENU pressed");  
        break;
        
        case INPUT_KEY_PLAY: 
            LOG_INF("PLAY pressed");  
        break;
        
        case INPUT_KEY_UP:   
            LOG_INF("UP pressed");    
        break;
        
        case INPUT_KEY_DOWN: 
            LOG_INF("DOWN pressed");  
        break;
    }
}
