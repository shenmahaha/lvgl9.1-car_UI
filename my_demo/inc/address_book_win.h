#ifndef __ADDRESS_BOOK_WIN_H // 定义以防止递归包含
#define __ADDRESS_BOOK_WIN_H

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


extern lv_obj_t * address_book_screen ;
extern void address_book_win_init(void);





#endif