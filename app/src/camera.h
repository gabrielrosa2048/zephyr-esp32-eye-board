#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <zephyr/kernel.h>

#define CAM_WIDTH   320
#define CAM_HEIGHT  240
#define MAX_FRAME_SIZE (CAM_WIDTH * CAM_HEIGHT * 2)

struct frame_msg {

    uint8_t *data;   
    size_t size;
};

extern struct k_msgq frame_queue;

#endif