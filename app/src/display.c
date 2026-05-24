#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

#include "camera.h"
#include "events.h"
#include "msgq.h"
#include "sntp.h"

LOG_MODULE_REGISTER(display, LOG_LEVEL_INF);

static uint8_t __attribute__((section(".ext_ram.bss"), aligned(CONFIG_VIDEO_BUFFER_POOL_ALIGN))) canvas_buf[CAM_WIDTH * CAM_HEIGHT * 2];

static const struct device *display;
static struct display_capabilities caps;

static lv_obj_t *canvas;
static lv_obj_t *status_bar;
static lv_obj_t *time_label;
static lv_obj_t *wifi_icon;
static lv_obj_t *ssid_label;

static struct k_timer timer_update;

static void ui_init();
static void ui_set_wifi();
static void ui_set_time(uint8_t hour, uint8_t minute, uint8_t second);

static void timer_update_handler(struct k_timer *timer);

void display_thread(){

    struct frame_msg msg;

    ui_init();
    
    k_timer_init(&timer_update, timer_update_handler, NULL);
    k_timer_start(&timer_update, K_SECONDS(1), K_SECONDS(1));

    while(1){
        
        ui_set_wifi();

        uint32_t events = k_event_wait(&app_events, EVENT_SNTP_SYNCED | EVENT_TIME_UPDATE, false, K_NO_WAIT);

        if((events & EVENT_SNTP_SYNCED) && (events & EVENT_TIME_UPDATE)){

            sntp_msg_t msg = sntp_get_current_time();
            ui_set_time(msg.hour, msg.min, msg.sec);

            k_event_clear(&app_events, EVENT_TIME_UPDATE);
        }

        if (!k_msgq_get(&frame_queue, &msg, K_NO_WAIT)) {

            LOG_INF("Camera frame received: %u bytes", msg.size);

            /* Cria canvas uma única vez */
            if (canvas == NULL) {
                canvas = lv_canvas_create(lv_scr_act());
                lv_canvas_set_buffer(canvas, canvas_buf,
                                    CAM_WIDTH, CAM_HEIGHT,
                                    LV_COLOR_FORMAT_RGB565);
                lv_obj_align_to(canvas, status_bar, LV_ALIGN_OUT_BOTTOM_MID, 0,
                (216 - CAM_HEIGHT) / 2);  /* centraliza no espaço restante */
            }

            memcpy(canvas_buf, msg.data, msg.size);
            lv_obj_invalidate(canvas);
            
        }

        lv_task_handler();        

        k_msleep(10);
    }
}

static void ui_init(){

    display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

    if (!device_is_ready(display)) {
        
        LOG_ERR("Display not found: %s", display->name);
        return;
    }

    display_get_capabilities(display, &caps);
    
    LOG_INF("Device name: %s", display->name);
    LOG_INF("Resolution: %dx%d", caps.x_resolution, caps.y_resolution);

    display_blanking_off(display);

    /* Background */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);

    /* Status bar */
    status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, 240, 24);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(status_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(status_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(status_bar, 0, LV_PART_MAIN);

    /* Hora — esquerda */
    time_label = lv_label_create(status_bar);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 6, 0);
    lv_label_set_text(time_label, "");

    /* Ícone WiFi — direita */
    wifi_icon = lv_label_create(status_bar);
    lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(wifi_icon, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(wifi_icon, LV_ALIGN_RIGHT_MID, -6, 0);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);

    /* Nome da rede — ao lado do ícone */
    ssid_label = lv_label_create(status_bar);
    lv_obj_set_style_text_color(ssid_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ssid_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align_to(ssid_label, wifi_icon, LV_ALIGN_OUT_LEFT_MID, -4, 0);
    lv_label_set_text(ssid_label, "");
}

void ui_set_wifi(){

    wifi_msg_t wifi_msg;

    if(k_msgq_get(&wifi_msgq, &wifi_msg, K_NO_WAIT) == 0){

        if(wifi_msg.type == WIFI_MSG_CONNECTED){

            lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  
            lv_obj_set_style_text_color(ssid_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_label_set_text(ssid_label, wifi_msg.ssid);
            
        } else {

            lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0x000000), LV_PART_MAIN);  
            lv_obj_set_style_text_color(ssid_label, lv_color_hex(0x000000), LV_PART_MAIN);
            lv_label_set_text(ssid_label, "");
        }
    }

    lv_obj_align_to(ssid_label, wifi_icon, LV_ALIGN_OUT_LEFT_MID, -4, 0);
}

static void ui_set_time(uint8_t hour, uint8_t min, uint8_t second){

    char buf[14];
    
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hour, min, second);
    
    lv_label_set_text(time_label, buf);
}

static void timer_update_handler(struct k_timer *timer){

    k_event_post(&app_events, EVENT_TIME_UPDATE);
}

static Z_KERNEL_STACK_DEFINE_IN(display_stack, 8192, __attribute__((section(".ext_ram.bss"))));

static struct k_thread display_thread_data;
k_tid_t display_tid;

static int display_thread_init(void){
    
    display_tid = k_thread_create(
        &display_thread_data,
        display_stack,
        K_THREAD_STACK_SIZEOF(display_stack),
        display_thread,
        NULL, NULL, NULL,
        8, 0, K_NO_WAIT
    );
    k_thread_name_set(display_tid, "display_tid");
    return 0;
}

SYS_INIT(display_thread_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);