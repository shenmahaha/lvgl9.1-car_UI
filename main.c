#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <../my_demo/inc/home_win.h>
static const char * getenv_default(const char * name, const char * dflt)
{
    return getenv(name) ?: dflt;
}

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    const char * device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t * disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    const char * device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t * disp = lv_linux_drm_create();

    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    const int width  = atoi(getenv("LV_SDL_VIDEO_WIDTH") ?: "800");
    const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ?: "480");

    lv_sdl_window_create(width, height);
}
#else
// #error Unsupported configuration
#endif

#define SDL_INPUT 1
#define LINUX_INPUT 0

pthread_mutex_t lock ;// 全局互斥锁
int main(void)
{
    lv_init();
    /*Linux display device init*/
    lv_linux_disp_init();

#if LINUX_INPUT
    // init LINUX_INPUT
    lv_indev_t * ts = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    lv_evdev_set_calibration(ts, 0, 0, 800, 480); // 蓝色边框屏幕校准
    // lv_evdev_set_calibration(ts,0,0,1024,600);//黑子边框屏幕校准
#endif

#if SDL_INPUT
    // init input device
    lv_sdl_mouse_create();
    lv_sdl_keyboard_create();
    lv_sdl_mousewheel_create();
#endif

    // ui_init();
     if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("互斥锁初始化失败\n");
        return 1;
    }
    home_win_init();

    /*Handle LVGL tasks*/
    while(1) {
        pthread_mutex_lock(&lock); // 上锁以防止竞态条件
        lv_timer_handler();
        pthread_mutex_unlock(&lock); // 解锁
        usleep(5000);
    }
    return 0;
}
