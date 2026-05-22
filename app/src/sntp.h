#ifndef _SNTP_H_
#define _SNTP_H_

#include <zephyr/net/sntp.h>

#define UTC_OFFSET_SECONDS  (-3 * 3600)  /* UTC-3 */

typedef struct {

    uint8_t hour;
    uint8_t min;
    uint8_t sec;

} sntp_msg_t;

void sntp_sync_time();

sntp_msg_t sntp_get_current_time();

#endif