cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(zephyr-esp32-eye-board)

target_sources(app PRIVATE
    
    src/camera.c
    src/display.c
    src/events.c
    src/led.c
    src/main.c
    src/msgq.c
    src/sntp.c
    src/wifi.c
)

target_include_directories(app PRIVATE

)
