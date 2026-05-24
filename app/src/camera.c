#include "camera.h"
#include <zephyr/device.h>
#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(camera, LOG_LEVEL_INF);

/* Buffer na PSRAM */
static uint8_t __attribute__((section(".ext_ram.bss"), aligned(CONFIG_VIDEO_BUFFER_POOL_ALIGN))) psram_frame[MAX_FRAME_SIZE];

K_MSGQ_DEFINE(frame_queue, sizeof(struct frame_msg), 1, 4);

static struct video_format fmt = {};

const struct device *camera_init(){

    const struct device *video_dev;
    enum video_buf_type type = VIDEO_BUF_TYPE_OUTPUT;
    struct video_buffer *buffers[1];
    int i = 0;

    /* Starting the device */
    video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
    if (!device_is_ready(video_dev)) {
        
        LOG_ERR("Camera not ready");
        return NULL;
    }
    LOG_INF("Camera: %s", video_dev->name);

    /* Configuring the format RGB565 320x240 */
	fmt.type = type;
	if (video_get_format(video_dev, &fmt)) {
		LOG_ERR("Unable to retrieve video format");
		return 0;
	}

    fmt.width = CAM_WIDTH;
	fmt.height = CAM_HEIGHT;
	fmt.pixelformat = VIDEO_PIX_FMT_RGB565;

    if (video_set_format(video_dev, &fmt)) {
		return 0;
	}

	LOG_INF("Format: %c%c%c%c %ux%u %u", (char)fmt.pixelformat, (char)(fmt.pixelformat >> 8),
		(char)(fmt.pixelformat >> 16), (char)(fmt.pixelformat >> 24), fmt.width, fmt.height,
		fmt.pitch);


    /* Allocating the video buffer */
    for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		
        buffers[i] = video_buffer_aligned_alloc(fmt.size, CONFIG_VIDEO_BUFFER_POOL_ALIGN, K_NO_WAIT);
		
        if (buffers[i] == NULL) {
			LOG_ERR("Unable to alloc video buffer");
			return 0;
		}
		
        buffers[i]->type = type;
		video_enqueue(video_dev, buffers[i]);
	}

    if (video_stream_start(video_dev, type)) {
		LOG_ERR("Unable to start capture (interface)");
		return 0;
	}

    LOG_INF("Stream started");
    return video_dev;
}

void camera_thread(){

    const struct device *video_dev = camera_init();

    struct video_buffer *vbuf;

    struct frame_msg msg;

    struct video_control rotate_video_ctrl = {
        .id  = VIDEO_CID_VFLIP,
        .val = 1
    };
    video_set_ctrl(video_dev, &rotate_video_ctrl);

    while (1) {

        if (video_dequeue(video_dev, &vbuf, K_MSEC(1000))) {
            LOG_WRN("Timeout");
            continue;
        }

        memcpy(psram_frame, vbuf->buffer, vbuf->bytesused);

        vbuf->bytesused = 0;
        video_enqueue(video_dev, vbuf);

        // LOG_INF("Frame: %u bytes", vbuf->bytesused);

        video_enqueue(video_dev, vbuf);

        msg.data = psram_frame;
        msg.size = MAX_FRAME_SIZE;
        k_msgq_put(&frame_queue, &msg, K_NO_WAIT);
    }
}

static Z_KERNEL_STACK_DEFINE_IN(camera_stack, 4096, __attribute__((section(".ext_ram.bss"))));

static struct k_thread camera_thread_data;
k_tid_t camera_tid;

static int camera_thread_init(void){

    camera_tid = k_thread_create(
        &camera_thread_data,
        camera_stack,
        K_THREAD_STACK_SIZEOF(camera_stack),
        camera_thread,
        NULL, NULL, NULL,
        5, 0, K_NO_WAIT
    );

    k_thread_name_set(camera_tid, "camera_tid");

    return 0;
}

SYS_INIT(camera_thread_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
