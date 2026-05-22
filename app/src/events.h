#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <zephyr/kernel.h>

#define EVENT_SNTP_SYNCED       BIT(0)
#define EVENT_TIME_UPDATE       BIT(1)

extern struct k_event app_events;

#endif