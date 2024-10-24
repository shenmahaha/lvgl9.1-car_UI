#ifndef __HOME_WIN_H // 定义以防止递归包含
#define __HOME_WIN_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "../lvgl/lvgl.h"
#include "../lvgl/examples/lv_examples.h"
#include <sys/types.h>
#include <dirent.h>
#include "../lvgl/src/libs/freetype/lv_freetype.h"
#include <cjson/cJSON.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

//虚拟机
#define IMG_PATH "A:/home/won/samba_share/lvgl9.1/lv_port_linux/my_demo/data/img"
#define TTF_PATH "/home/won/samba_share/lvgl9.1/lv_port_linux/my_demo/data/ttf"
#define MP3_PATH "/home/won/samba_share/lvgl9.1/lv_port_linux/my_demo/data/mp3"
#define MP4_PATH "/home/won/samba_share/lvgl9.1/lv_port_linux/my_demo/data/mp4"
#define LRC_PATH "/home/won/samba_share/lvgl9.1/lv_port_linux/my_demo/data/lrc"

//开发板
// #define IMG_PATH "A:/root/user/data/img"
// #define TTF_PATH "/root/user/data/ttf"
// #define MP3_PATH "/root/user/data/mp3"
// #define MP4_PATH "/root/user/data/mp4"
// #define LRC_PATH "/root/user/data/lrc"

extern void home_win_init(void);

extern lv_obj_t * root;                  // 主页面
extern lv_obj_t * bg_img;                // 背景图片
extern lv_obj_t * time_label;            // 左上角时间
extern lv_obj_t * speed_label;           // 速度
extern lv_obj_t * mileage_label_val;     // 里程
extern lv_obj_t * electricity_label_val; // 电量

extern lv_obj_t * home_music_name_label; 
extern lv_obj_t * home_music_singer_label;
extern lv_obj_t * home_music_img;
extern lv_obj_t * music_start_icon;
extern lv_obj_t * music_stop_icon;

extern lv_obj_t *home_music_prev;
extern lv_obj_t * home_music_play;
extern lv_obj_t * home_music_pause;
extern lv_obj_t * home_music_next;



#endif