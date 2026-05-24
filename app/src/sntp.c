#include "sntp.h"

#include <zephyr/logging/log.h>

#include <time.h>

#include "events.h"

LOG_MODULE_REGISTER(sntp, LOG_LEVEL_INF);

void sntp_sync_time(void){

    struct sntp_time sntp_time;
    int ret;

    LOG_INF("Sntp sync started");

    ret = sntp_simple("pool.ntp.org", 10000, &sntp_time);

    if (ret != 0) {

        LOG_WRN("DNS failed, trying again..");

        ret = sntp_simple("200.160.7.186", 10000, &sntp_time);
    }

    if (ret != 0) {

        LOG_ERR("SNTP failed: %d", ret);

        return;  
    }

    struct timespec ts = {

        .tv_sec  = (time_t)(sntp_time.seconds) + UTC_OFFSET_SECONDS,
        .tv_nsec = 0,
    };

    clock_settime(CLOCK_REALTIME, &ts);

    struct tm tm_info;
    gmtime_r(&ts.tv_sec, &tm_info);

    k_event_post(&app_events, EVENT_SNTP_SYNCED);

    char time_str[72];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d %02d/%02d/%04d", tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec, tm_info.tm_mday, tm_info.tm_mon + 1, tm_info.tm_year + 1900);
    
    LOG_INF("Sntp synchronized: %s", time_str);    
    
    return;
}

sntp_msg_t sntp_get_current_time(){

    sntp_msg_t msg;

    struct timespec ts;
    
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm *t = gmtime(&ts.tv_sec);
    msg.hour = t->tm_hour;
    msg.min = t->tm_min;
    msg.sec = t->tm_sec;

    return msg;
}