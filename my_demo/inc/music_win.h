#ifndef __MUSIC_WIN_H // 定义以防止递归包含
#define __MUSIC_WIN_H

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
#include <math.h>

extern lv_obj_t * music_list;
extern lv_obj_t * music_screen;

extern void music_win_init(void);
extern char * music_name_list[1000]; // 存放音乐文件名
extern int music_count;   // 音乐数量
extern int music_index ;   // 当前播放音乐索引
// 播放状态
extern int music_state ; // 0:未开始 1:播放中 2:暂停
extern lv_obj_t * music_play;
extern lv_obj_t * music_pause;

extern void * music_btn_prev_click_event_cb(lv_event_t * e);
extern void * music_btn_next_click_event_cb(lv_event_t * e);
extern void * music_btn_play_click_event_cb(lv_event_t * e);
extern void * music_btn_pause_click_event_cb(lv_event_t * e);

#endif