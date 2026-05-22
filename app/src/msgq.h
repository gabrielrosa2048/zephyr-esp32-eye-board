#ifndef _MSGQ_H_
#define _MSGQ_H_

#define WIFI_SSID_MAX 33

#include <zephyr/kernel.h> 

typedef enum {
    
    WIFI_MSG_CONNECTED,
    WIFI_MSG_DISCONNECTED,

} wifi_msg_type_t;

typedef struct {
    
    wifi_msg_type_t type;
    char ssid[WIFI_SSID_MAX];

} wifi_msg_t;

extern struct k_msgq wifi_msgq;

#endif