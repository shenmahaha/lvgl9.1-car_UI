#ifndef __MAP_WIN_H // 定义以防止递归包含
#define __MAP_WIN_H

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern void map_win_init(); // 地图窗口初始化
#endif