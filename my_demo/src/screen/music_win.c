#include "../inc/music_win.h"
#include "../inc/home_win.h"
#include "../inc/movie.h"
#include "../inc/thread_pool.h"

void total_time_convert();
void cut_time_convert();
void * back_btn_event_cb(lv_event_t * e);
void * video_convert_btn_event_cb(lv_event_t * e);
void * music_convert_btn_event_cb(lv_event_t * e);

void * music_btn_prev_click_event_cb(lv_event_t * e);
void * music_btn_next_click_event_cb(lv_event_t * e);
void * music_btn_play_click_event_cb(lv_event_t * e);
void * music_btn_pause_click_event_cb(lv_event_t * e);

lv_obj_t * music_list;
lv_obj_t * video_list;
lv_obj_t * music_screen;
lv_obj_t * music_img;
lv_obj_t * container;
lv_obj_t * music_name;
lv_obj_t * music_singer;
lv_obj_t * music_prev;
lv_obj_t * music_next;
lv_obj_t * music_play;
lv_obj_t * music_pause;
lv_obj_t * total_music_time_label;
lv_obj_t * cut_music_time_label;
lv_obj_t * slider;
lv_obj_t * volume_slider;
lv_obj_t * video_btn;
lv_obj_t * music_btn;

// 歌词
lv_obj_t * music_lyric1;
lv_obj_t * music_lyric2;
lv_obj_t * music_lyric3;
lv_obj_t * music_lyric4;
lv_obj_t * music_lyric5;
lv_obj_t * music_lyric6;
lv_obj_t * music_lyric7;
lv_obj_t * lrc_container; // 歌词容器
char lrcfile[1024][1024] = {0};
int total_line           = 0; // 行数
int cut_line             = 4; // 行数

extern pthread_mutex_t lock;

char * music_name_list[1000] = {0}; // 存放音乐文件名
int music_count              = 0;   // 音乐数量
int music_index              = 0;   // 当前播放音乐索引

char * video_name_list[1000] = {0}; // 存放视频文件名
int video_count              = 0;   // 视频数量
int video_index              = 0;   // 当前播放视频索引

float total_music_time    = 0; // 总时长
int total_minutes         = 0;
float total_seconds       = 0;
float cut_music_time      = 0; // 当前时长
int cut_minutes           = 0;
float cut_seconds         = 0;
static thread_pool_p pool = NULL; // 线程池
int32_t music_volume      = 70;

int music_pthread_flag = 0; // 音乐线程已创建标志
int video_pthread_flag = 0; // 视频线程已创建标志
int lrc_pthread_flag   = 0; // 歌词线程已创建标志

// 播放状态
int music_state = 0; // 0:未开始 1:播放中 2:暂停
int video_state = 0; // 0:未开始 1:播放中 2:暂停

// 获取当前音乐时间线程
void * get_cut_music_time(void * arg)
{

    while(1) {

        if(music_state == 1) {
            cut_music_time = get_movie_position();
            // cut_music_time++;
            cut_time_convert();
            char cut_time_str[20] = {0};
            float val = (cut_music_time / total_music_time) * 1000;
            // 上锁
            pthread_mutex_lock(&lock);
            lv_label_set_text_fmt(cut_music_time_label, "%s", cut_time_str);
            lv_slider_set_value(slider, val, LV_ANIM_OFF);
            // 解锁
            pthread_mutex_unlock(&lock);

            if(cut_music_time >= total_music_time || cut_music_time < 0) {
                printf("播放结束\n");
                cut_music_time = 0;
                // 上锁
                pthread_mutex_lock(&lock);
                lv_label_set_text_fmt(cut_music_time_label, "%s", "00:00");
                lv_event_t e;
                e.code           = LV_EVENT_CLICKED;
                e.current_target = music_next;
                music_btn_next_click_event_cb(&e);
                pthread_mutex_unlock(&lock);
            }
        } else if(music_state == 2) {
        }

        usleep(10 * 1000);
    }
}
void * get_cut_video_time(void * arg)
{

    while(1) {

        if(video_state == 1) {

            cut_music_time = get_movie_position();
            // cut_music_time++;
            cut_time_convert();
            char cut_time_str[20] = {0};
            float val = (cut_music_time / total_music_time) * 1000;
            // 上锁
            pthread_mutex_lock(&lock);
            lv_label_set_text_fmt(cut_music_time_label, "%s", cut_time_str);
            lv_slider_set_value(slider, val, LV_ANIM_OFF);
            // 解锁
            pthread_mutex_unlock(&lock);

            if(cut_music_time >= total_music_time || cut_music_time < 0) {
                printf("播放结束\n");
                cut_music_time = 0;
                // 上锁
                pthread_mutex_lock(&lock);
                lv_label_set_text_fmt(cut_music_time_label, "%s", "00:00");
                lv_event_t e;
                e.code           = LV_EVENT_CLICKED;
                e.current_target = music_next;
                music_btn_next_click_event_cb(&e);
                pthread_mutex_unlock(&lock);
            }

        } else if(video_state == 2) {
        }

        usleep(100 * 1000);
    }
}

void * get_lrc_task(void * arg)
{

    while(1) {
        usleep(100 * 1000);
        if(music_state == 1 && cut_line < total_line - 3) {
            char time_str[20]; // 存储提取出来的时间部分
            int minutes;
            float seconds, total_seconds;

            // 找到 '[' 和 ']' 的位置
            char * start = strchr(lrcfile[cut_line], '[');
            char * end   = strchr(lrcfile[cut_line], ']');

            if(start != NULL && end != NULL && end > start) {
                // 计算时间戳的长度并提取
                int len = end - start - 1;         // 不包括 '[' 和 ']'
                strncpy(time_str, start + 1, len); // 跳过 '['
                time_str[len] = '\0';              // 添加字符串结尾的 '\0'

                // 解析时间，格式为 MM:SS.mmm
                sscanf(time_str, "%d:%f", &minutes, &seconds);

                // 转换成总秒数
                total_seconds = minutes * 60 + seconds;

                // printf("时间部分: %s\n", time_str);
                // printf("总秒数: %.3f 秒\n", total_seconds);
                // printf("当前播放时间: %.3f 秒\n", cut_music_time);
                // 上锁
                pthread_mutex_lock(&lock);
                if(cut_music_time >= total_seconds) {
                    char str[1024] = {0};
                    strcpy(str, lrcfile[cut_line - 3]);
                    char * start = strchr(str, ']');
                    start++;
                    start[strlen(start) - 1] = '\0';
                    lv_label_set_text(music_lyric1, start);

                    strcpy(str, lrcfile[cut_line - 2]);
                    start = strchr(str, ']');
                    start++;
                    start[strlen(start) - 1] = '\0';
                    lv_label_set_text(music_lyric2, start);

                    strcpy(str, lrcfile[cut_line - 1]);
                    start = strchr(str, ']');
                    start++;
                    start[strlen(start) - 1] = '\0';
                    lv_label_set_text(music_lyric3, start);

                    if(lrcfile[cut_line] != NULL) {
                        strcpy(str, lrcfile[cut_line]);
                        start = strchr(str, ']');
                        start++;
                        start[strlen(start) - 1] = '\0';
                        lv_label_set_text(music_lyric4, start);
                    }

                    if(lrcfile[cut_line + 1] != NULL) {
                        strcpy(str, lrcfile[cut_line + 1]);
                        start = strchr(str, ']');
                        start++;
                        start[strlen(start) - 1] = '\0';
                        lv_label_set_text(music_lyric5, start);
                    }

                    if(lrcfile[cut_line + 2] != NULL) {
                        strcpy(str, lrcfile[cut_line + 2]);
                        start = strchr(str, ']');
                        start++;
                        start[strlen(start) - 1] = '\0';
                        lv_label_set_text(music_lyric6, start);
                    }

                    if(lrcfile[cut_line + 3] != NULL) {
                        strcpy(str, lrcfile[cut_line + 3]);
                        start = strchr(str, ']');
                        start++;
                        start[strlen(start) - 1] = '\0';
                        lv_label_set_text(music_lyric7, start);
                        cut_line++;
                    }
                }
                // 解锁
                pthread_mutex_unlock(&lock);
            } else {
                printf("未找到有效的时间戳\n");
            }
        }
    }
}

// 快速跳转歌词
void jump_to_lyrics()
{
    for(int i = 4; i < total_line - 3; i++) {
        
        char time_str[20]; // 存储提取出来的时间部分
        int minutes;
        float seconds, total_seconds;

        // 找到 '[' 和 ']' 的位置
        char * start = strchr(lrcfile[i], '[');
        char * end   = strchr(lrcfile[i], ']');

        if(start != NULL && end != NULL && end > start) {
            // 计算时间戳的长度并提取
            int len = end - start - 1;         // 不包括 '[' 和 ']'
            strncpy(time_str, start + 1, len); // 跳过 '['
            time_str[len] = '\0';              // 添加字符串结尾的 '\0'

            // 解析时间，格式为 MM:SS.mmm
            sscanf(time_str, "%d:%f", &minutes, &seconds);

            // 转换成总秒数
            total_seconds = minutes * 60 + seconds;

            if(cut_music_time-total_seconds<0.05){
                cut_line = i;
                break;
            }
        }
    }
}

// 上一首按钮回调函数
void * music_btn_prev_click_event_cb(lv_event_t * e)
{

    lv_obj_t * obj = lv_event_get_target(e);
    LV_UNUSED(obj);
    printf("Clicked: %s\n", "prev");
    // 播放上一首音乐
    if(music_state != 0) {
        if(music_index != 0) {
            if(music_state != 0) {
                quit_movie();
                music_state = 0;
                usleep(100 * 1000);
            }
            char path[1024];
            music_index--;
            sprintf(path, MP3_PATH "/%s", music_name_list[music_index]);
            start_movie(0, 0, 800, 480, path);

            cut_music_time = 0; // 重新开始计时
            lv_label_set_text_fmt(cut_music_time_label, "%s", "00:00");
            get_total_music_time();
            // 获取当前音乐总时长
            while((int)total_music_time == -1) {
                start_movie(0, 0, 800, 480, path);
                get_total_music_time();
                usleep(100 * 1000);
            }
            get_lyric();
            music_state = 1;

            lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
            // 回显画面
            lv_obj_t * tar_btn = lv_obj_get_child(music_list, music_index + 1); // 获取当前子对象
            lv_obj_set_style_bg_color(tar_btn, lv_color_hex(0x4B8BFF), 0);
            lv_obj_t * prev_btn = lv_obj_get_child(music_list, music_index + 2); // 获取上一个子对象
            lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x101429), 0);      // 将其他按钮设置为蓝色

            char img_path[1024];
            char name[1024];
            sscanf(music_name_list[music_index], "%[^.]", name);
            sprintf(img_path, IMG_PATH "/%s.bmp", name);
            printf("img_path:%s\n", img_path);
            lv_img_set_src(music_img, img_path);
            char singer[50], song[50];
            if(strchr(music_name_list[music_index], '-') != NULL) {
                sscanf(music_name_list[music_index], "%[^-]-%[^.]", singer, song);
                lv_label_set_text(music_name, song);
                lv_label_set_text(music_singer, singer);
                lv_label_set_text(home_music_name_label, song);
                lv_label_set_text(home_music_singer_label, singer);
            } else {
                lv_label_set_text(music_name, name);
                lv_label_set_text(music_singer, "-未知");
                lv_label_set_text(home_music_name_label, name);
                lv_label_set_text(home_music_singer_label, "-未知");
            }
            // 回显主页面信息
            char home_img_path[1024];
            sprintf(home_img_path, IMG_PATH "/%s.bmp", name);
            lv_img_set_src(home_music_img, img_path);
            lv_obj_add_flag(home_music_play, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(home_music_pause, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(video_state != 0) {
        if(video_index != 0) {
            if(video_state != 0) {
                quit_movie();
                video_state = 0;
                usleep(100 * 1000);
            }
            char path[1024];
            video_index--;
            sprintf(path, MP4_PATH "/%s", video_name_list[video_index]);
            // TODO
            start_movie(200, 5, 595, 355, path);

            cut_music_time = 0; // 重新开始计时
            lv_label_set_text_fmt(cut_music_time_label, "%s", "00:00");
            get_total_music_time();
            // 获取当前音乐总时长
            while((int)total_music_time == -1) {
                // TODO
                start_movie(200, 5, 595, 355, path);
                get_total_music_time();
                usleep(100 * 1000);
            }
            video_state = 1;

            lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
            // 回显画面
            lv_obj_t * tar_btn = lv_obj_get_child(video_list, video_index + 1); // 获取当前子对象
            lv_obj_set_style_bg_color(tar_btn, lv_color_hex(0x4B8BFF), 0);
            lv_obj_t * prev_btn = lv_obj_get_child(video_list, video_index + 2); // 获取上一个子对象
            lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x101429), 0);      // 将其他按钮设置为蓝色

            char img_path[1024];
            char name[1024];
            sscanf(video_name_list[music_index], "%[^.]", name);
            sprintf(img_path, IMG_PATH "/%s.bmp", name);
            printf("img_path:%s\n", img_path);
        }
    }
}

// 下一首按钮回调函数
void * music_btn_next_click_event_cb(lv_event_t * e)
{

    lv_obj_t * obj = lv_event_get_target(e);
    LV_UNUSED(obj);
    if(music_state != 0) {
        if(music_index != music_count - 1) {
            if(music_state != 0) {
                quit_movie();
                music_state = 0;
                usleep(100 * 1000);
            }
            char path[1024];
            music_index++;
            sprintf(path, MP3_PATH "/%s", music_name_list[music_index]);
            start_movie(0, 0, 800, 480, path);

            cut_music_time = 0; // 重新开始计时
            lv_label_set_text_fmt(cut_music_time_label, "%s", "00:00");
            get_total_music_time();
            // 获取当前音乐总时长
            while((int)total_music_time == -1) {
                start_movie(0, 0, 800, 480, path);
                get_total_music_time();
                usleep(100 * 1000);
            }
            get_lyric();
            music_state = 1;

            lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
            // 回显画面
            lv_obj_t * tar_btn = lv_obj_get_child(music_list, music_index + 1); // 获取当前子对象
            lv_obj_set_style_bg_color(tar_btn, lv_color_hex(0x4B8BFF), 0);
            lv_obj_t * prev_btn = lv_obj_get_child(music_list, music_index); // 获取上一个子对象
            lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x101429), 0);  // 将其他按钮设置为蓝色

            char img_path[1024];
            char name[1024];
            sscanf(music_name_list[music_index], "%[^.]", name);
            sprintf(img_path, IMG_PATH "/%s.bmp", name);
            printf("img_path:%s\n", img_path);
            lv_img_set_src(music_img, img_path);
            char singer[50], song[50];
            if(strchr(music_name_list[music_index], '-') != NULL) {
                sscanf(music_name_list[music_index], "%[^-]-%[^.]", singer, song);
                lv_label_set_text(music_name, song);
                lv_label_set_text(music_singer, singer);
                lv_label_set_text(home_music_name_label, song);
                lv_label_set_text(home_music_singer_label, singer);
            } else {
                lv_label_set_text(music_name, name);
                lv_label_set_text(music_singer, "-未知");
                lv_label_set_text(home_music_name_label, name);
                lv_label_set_text(home_music_singer_label, "-未知");
            }
            // 回显主页面信息
            char home_img_path[1024];
            sprintf(home_img_path, IMG_PATH "/%s.bmp", name);
            lv_img_set_src(home_music_img, img_path);
            lv_obj_add_flag(home_music_play, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(home_music_pause, LV_OBJ_FLAG_HIDDEN);
        } else {
            music_state = 2;
            lv_obj_add_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(music_play, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if(video_state != 0) {
        if(video_index != video_count - 1) {
            if(video_state != 0) {
                quit_movie();
                video_state = 0;
                usleep(100 * 1000);
            }
            char path[1024];
            video_index++;
            sprintf(path, MP4_PATH "/%s", video_name_list[video_index]);
            printf("path:%s\n\n", path);
            // TODO
            start_movie(200, 5, 595, 355, path);

            cut_music_time = 0; // 重新开始计时
            lv_label_set_text_fmt(cut_music_time_label, "%s", "00:00");
            get_total_music_time();
            // 获取当前音乐总时长
            while((int)total_music_time == -1) {
                // TODO
                start_movie(200, 5, 595, 355, path);
                get_total_music_time();
                usleep(100 * 1000);
            }
            video_state = 1;

            lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
            // 回显画面
            lv_obj_t * tar_btn = lv_obj_get_child(video_list, video_index + 1); // 获取当前子对象
            lv_obj_set_style_bg_color(tar_btn, lv_color_hex(0x4B8BFF), 0);
            lv_obj_t * prev_btn = lv_obj_get_child(video_list, video_index); // 获取上一个子对象
            lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x101429), 0);  // 将其他按钮设置为蓝色

        } else {
            video_state = 2;
            lv_obj_add_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(music_play, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// 播放按钮回调函数
void * music_btn_play_click_event_cb(lv_event_t * e)
{

    printf("Clicked: %s\n", "play");
    if(music_state != 0) {

        music_state = 1;
        lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(home_music_play, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(home_music_pause, LV_OBJ_FLAG_HIDDEN);
        // 播放音频
        stop_movie();
    }
    if(video_state != 0) {
        video_state = 1;
        lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
        stop_movie();
    }
}
// 暂停按钮回调函数
void * music_btn_pause_click_event_cb(lv_event_t * e)
{

    printf("Clicked: %s\n", "pause");
    if(music_state != 0) {
        lv_obj_add_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(music_play, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(home_music_pause, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(home_music_play, LV_OBJ_FLAG_HIDDEN);
        // 暂停音频
        stop_movie();
        music_state = 2;
    }
    if(video_state != 0) {
        lv_obj_add_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(music_play, LV_OBJ_FLAG_HIDDEN);
        // 暂停音频
        stop_movie();
        video_state = 2;
    }
}
// 列表回调函数
void * music_btn_event_cb(lv_event_t * e)
{

    lv_obj_t * obj = lv_event_get_target(e);
    LV_UNUSED(obj);
    printf("Clicked: %s\n", lv_list_get_button_text(music_list, obj));
    char str[1024];
    strcpy(str, lv_list_get_button_text(music_list, obj));
    char * name     = strtok(str, ".");
    char path[1024] = {0};
    sprintf(path, IMG_PATH "/%s.bmp", name);
    // 设置图片
    lv_img_set_src(music_img, path);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x4B8BFF), 0);
    // 获取按钮所属的父对象（列表）
    lv_obj_t * list = lv_obj_get_parent(obj);

    // 遍历列表中的所有子对象（即按钮）
    lv_obj_t * child;
    lv_obj_t * other_btn;
    lv_obj_t * first_child = lv_obj_get_child(list, 0); // 获取列表的第一个子对象
    child                  = first_child;

    while(child != NULL) {
        // 如果子对象是按钮且不是当前按下的按钮
        if(child != obj) {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x101429), 0); // 将其他按钮设置为蓝色
        }
        child = lv_obj_get_child(list, lv_obj_get_index(child) + 1); // 获取下一个子对象
    }

    if(strstr(name, "-") == NULL) {
        lv_label_set_text(music_name, name);
        lv_label_set_text(music_singer, "-未知");
    } else {
        char * music_singer_str = strtok(name, "-");
        lv_label_set_text(music_singer, music_singer_str);
        lv_label_set_text_fmt(music_singer, "-%s", music_singer_str);

        char * music_name_str = strtok(NULL, "-");
        lv_label_set_text_fmt(music_name, "%s", music_name_str);
    }
    lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
    if(music_state != 0) {
        quit_movie();
        music_state = 0;
        usleep(100 * 1000); // 休眠300ms
    }
    char * imp_path[1024];
    strcpy(str, lv_list_get_button_text(music_list, obj));
    sprintf(imp_path, MP3_PATH "/%s", str);
    start_movie(0, 0, 800, 480, imp_path);
    cut_music_time = 0; // 重新开始计时
    lv_label_set_text_fmt(cut_music_time_label, "%s", "00:00");
    // 获取当前音乐总时长
    get_total_music_time();
    while(total_music_time == -1) {
        start_movie(0, 0, 800, 480, imp_path);
        get_total_music_time();
        usleep(100 * 1000);
    }

    if(music_state == 0 && music_pthread_flag == 0) {
        THREAD_POOL_AddTask(pool, get_cut_music_time, NULL);
        music_pthread_flag = 1;
    }

    for(int i = 0; i < music_count; i++) {
        if(strcmp(music_name_list[i], str) == 0) {
            music_index = i;
            break;
        }
    }
    if(music_state == 0 && lrc_pthread_flag == 0) {
        THREAD_POOL_AddTask(pool, get_lrc_task, NULL);
        lrc_pthread_flag = 1;
    }

    music_state = 1;
    // 获取歌词
    get_lyric();
}

// 视频列表回调函数
void video_btn_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    LV_UNUSED(obj);
    printf("Clicked: %s\n", lv_list_get_button_text(video_list, obj));
    char str[1024];
    strcpy(str, lv_list_get_button_text(video_list, obj));
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x4B8BFF), 0);
    // 获取按钮所属的父对象（列表）
    lv_obj_t * list = lv_obj_get_parent(obj);

    // 遍历列表中的所有子对象（即按钮）
    lv_obj_t * child;
    lv_obj_t * other_btn;
    lv_obj_t * first_child = lv_obj_get_child(list, 0); // 获取列表的第一个子对象
    child                  = first_child;

    while(child != NULL) {
        // 如果子对象是按钮且不是当前按下的按钮
        if(child != obj) {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x101429), 0); // 将其他按钮设置为蓝色
        }
        child = lv_obj_get_child(list, lv_obj_get_index(child) + 1); // 获取下一个子对象
    }

    lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
    if(video_state != 0) {
        quit_movie();
        video_state = 0;
        music_state = 0;
        usleep(100 * 1000); // 休眠300ms
    }
    char * imp_path[1024];
    strcpy(str, lv_list_get_button_text(video_list, obj));
    sprintf(imp_path, MP4_PATH "/%s", str);
    printf("imp_path:%s\n\n", imp_path);
    // TODO
    start_movie(200, 5, 595, 355, imp_path);
    cut_music_time = 0; // 重新开始计时
    lv_label_set_text_fmt(cut_music_time_label, "%s", "00:00");
    // 获取当前音乐总时长
    get_total_music_time();
    while(total_music_time == -1) {
        // TODO
        start_movie(200, 5, 595, 355, imp_path);
        get_total_music_time();
        usleep(100 * 1000);
    }

    if(video_state == 0 && video_pthread_flag == 0) {
        THREAD_POOL_AddTask(pool, get_cut_video_time, NULL);
        video_pthread_flag = 1;
    }
    video_state = 1;
    for(int i = 0; i < video_count; i++) {
        if(strcmp(video_name_list[i], str) == 0) {
            video_index = i;
            break;
        }
    }
}
void music_win_init(void)
{

    lv_obj_t * font_music_list = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 20, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_music_list) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    lv_obj_t * font_music_name = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 30, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_music_name) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    lv_obj_t * font_cut_music_time = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 15, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_cut_music_time) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    lv_obj_t * font_music_lrc = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 21, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_music_lrc) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    open_mplayer_fifo();
    // 初始化线程池
    pool = malloc(sizeof(thread_pool));
    THREAD_POOL_Init(pool, 5);

    music_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(music_screen, 800, 480);
    lv_obj_set_style_pad_all(music_screen, 0, 0); // 移除所有的内边距
    lv_obj_remove_flag(music_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(music_screen, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条
    // 设置背景颜色
    lv_obj_set_style_bg_color(music_screen, lv_color_hex(0x181831), 0);

    lv_obj_add_flag(music_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_screen, LV_OBJ_FLAG_HIDDEN);
    // 创建音乐列表
    music_list = lv_list_create(music_screen);
    lv_obj_set_size(music_list, 200, 400);
    lv_obj_align(music_list, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_pad_all(music_list, 5, 0); // 移除所有的内边距
    lv_obj_set_style_bg_color(music_list, lv_color_hex(0x101429), 0);
    // 去除边框
    lv_obj_set_style_border_width(music_list, 0, 0);

    lv_obj_t * list_item = lv_list_add_text(music_list, "   音乐列表");
    lv_obj_set_style_text_font(music_list, font_music_list, 0);
    lv_obj_set_style_text_color(list_item, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(list_item, lv_color_hex(0x101429), 0);

    // 重置

    music_count = 0;
    video_count = 0;
    // 读取音乐文件
    struct dirent * entry;
    DIR * dp; // 打开当前目录

    dp = opendir(MP3_PATH);
    if(dp == NULL) {
        perror("opendir");
    }

    // 循环读取目录内容
    while((entry = readdir(dp)) != NULL) {
        // 跳过 "." 和 ".."
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // 打印文件或目录名称

        const char * ext = strrchr(entry->d_name, '.');
        printf("Found file or directory: %s\n", entry->d_name);
        if(ext != NULL && strcmp(ext, ".mp3") == 0) {
            lv_obj_t * btn = lv_list_add_button(music_list, NULL, entry->d_name);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x101429), 0);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x4B8BFF), LV_STATE_PRESSED); // 红色背景，按下时生效
            lv_obj_add_event_cb(btn, music_btn_event_cb, LV_EVENT_CLICKED, NULL);
            music_name_list[music_count] = (char *)malloc(strlen(entry->d_name) + 1);
            strcpy(music_name_list[music_count], entry->d_name);
            music_count++;
        }
    }
    // 关闭目录
    closedir(dp);

    // 创建视频列表
    video_list = lv_list_create(music_screen);
    lv_obj_set_size(video_list, 200, 400);
    lv_obj_align(video_list, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_pad_all(video_list, 5, 0); // 移除所有的内边距
    lv_obj_set_style_bg_color(video_list, lv_color_hex(0x101429), 0);
    // 去除边框
    lv_obj_set_style_border_width(video_list, 0, 0);

    lv_obj_t * list_v_item = lv_list_add_text(video_list, "   视频列表");
    lv_obj_set_style_text_font(video_list, font_music_list, 0);
    lv_obj_set_style_text_color(list_v_item, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(list_v_item, lv_color_hex(0x101429), 0);

    // 读取视频文件
    struct dirent * v_entry;
    DIR * v_dp; // 打开当前目录

    v_dp = opendir(MP4_PATH);
    if(v_dp == NULL) {
        perror("opendir");
    }

    // 循环读取目录内容
    while((v_entry = readdir(v_dp)) != NULL) {
        // 跳过 "." 和 ".."
        if(strcmp(v_entry->d_name, ".") == 0 || strcmp(v_entry->d_name, "..") == 0) {
            continue;
        }
        // 打印文件或目录名称

        const char * ext = strrchr(v_entry->d_name, '.');
        printf("Found file or directory: %s\n", v_entry->d_name);
        if(ext != NULL && (strcmp(ext, ".mp4") == 0 || strcmp(ext, ".avi") == 0)) {
            lv_obj_t * btn = lv_list_add_button(video_list, NULL, v_entry->d_name);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x101429), 0);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x4B8BFF), LV_STATE_PRESSED);
            lv_obj_add_event_cb(btn, video_btn_event_cb, LV_EVENT_CLICKED, NULL);
            video_name_list[video_count] = (char *)malloc(strlen(v_entry->d_name) + 1);
            strcpy(video_name_list[video_count], v_entry->d_name);
            video_count++;
        }
    }
    // 关闭目录
    closedir(dp);

    lv_obj_add_flag(music_list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(video_list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_list, LV_OBJ_FLAG_HIDDEN);

    static lv_style_t style;
    lv_style_init(&style);
    /*Add a shadow*/
    lv_style_set_shadow_width(&style, 40);
    lv_style_set_shadow_color(&style, lv_color_black());

    // 创建一个容器对象作为圆形裁剪区域
    container = lv_obj_create(music_screen);
    lv_obj_set_size(container, 250, 250);                    // 设置容器大小为正方形
    lv_obj_set_style_radius(container, LV_RADIUS_CIRCLE, 0); // 设置圆角为圆形
    lv_obj_set_style_clip_corner(container, true, 0);   // 启用裁剪，确保超出容器区域的内容被剪掉
    lv_obj_align(container, LV_ALIGN_TOP_MID, -30, 20); // 使容器居中或根据需要设置位置
    lv_obj_set_style_pad_all(container, 0, 0);          // 移除所有的内边距
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条

    lv_obj_add_style(container, &style, 0);

    // 将图片添加到容器中
    music_img = lv_img_create(container);
    lv_img_set_src(music_img,
                   IMG_PATH"/music_bg.bmp"); // 替换为你的图片源
    lv_obj_align(music_img, LV_ALIGN_CENTER, 0, 0); // 将图片居中对齐

    // 根据需要调整图片大小
    lv_obj_set_size(music_img, 250, 250); // 使图片大小与容器匹配，保持为正方形

    // 显示音乐名称
    music_name = lv_label_create(music_screen);
    lv_label_set_text(music_name, "---");
    lv_label_set_long_mode(music_name, LV_LABEL_LONG_SCROLL_CIRCULAR); /*Circular scroll*/
    lv_obj_align(music_name, LV_ALIGN_TOP_MID, -30, 290);
    lv_obj_set_style_text_font(music_name, font_music_name, 0);
    lv_obj_set_width(music_name, 250);
    // 让文本居中
    lv_obj_set_style_text_align(music_name, LV_TEXT_ALIGN_CENTER, 0);
    // 字体颜色
    lv_obj_set_style_text_color(music_name, lv_color_hex(0xFFFFFF), 0);

    // 显示歌手
    music_singer = lv_label_create(music_screen);
    lv_label_set_text(music_singer, "-");
    lv_label_set_long_mode(music_singer, LV_LABEL_LONG_SCROLL_CIRCULAR); /*Circular scroll*/
    lv_obj_align(music_singer, LV_ALIGN_TOP_MID, -30, 335);
    lv_obj_set_style_text_font(music_singer, font_music_list, 0);
    lv_obj_set_width(music_singer, 250);
    // 让文本居中
    lv_obj_set_style_text_align(music_singer, LV_TEXT_ALIGN_CENTER, 0);
    // 字体颜色
    lv_obj_set_style_text_color(music_singer, lv_color_hex(0xFFFFFF), 0);

    // 显示上一首按钮
    music_prev = lv_img_create(music_screen);
    lv_img_set_src(music_prev, IMG_PATH "/btn_prev.png");
    lv_obj_align(music_prev, LV_ALIGN_BOTTOM_MID, -110, -55);
    // 设置图片为可点击
    lv_obj_add_flag(music_prev, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(music_prev, music_btn_prev_click_event_cb, LV_EVENT_CLICKED, NULL);

    // 播放按钮
    music_play = lv_img_create(music_screen);
    lv_img_set_src(music_play, IMG_PATH "/btn_play.png");
    lv_obj_align(music_play, LV_ALIGN_BOTTOM_MID, 100, -47);
    // 设置图片为可点击
    lv_obj_add_flag(music_play, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(music_play, music_btn_play_click_event_cb, LV_EVENT_CLICKED, NULL);

    // 暂停按钮
    music_pause = lv_img_create(music_screen);
    lv_img_set_src(music_pause, IMG_PATH "/btn_pause.png");
    lv_obj_align(music_pause, LV_ALIGN_BOTTOM_MID, 100, -47);
    // 设置图片为可点击
    lv_obj_add_flag(music_pause, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(music_pause, music_btn_pause_click_event_cb, LV_EVENT_CLICKED, NULL);

    // 添加隐藏属性
    lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_play, LV_OBJ_FLAG_HIDDEN);

    // 显示下一首按钮
    music_next = lv_img_create(music_screen);
    lv_img_set_src(music_next, IMG_PATH "/btn_next.png");
    lv_obj_align(music_next, LV_ALIGN_BOTTOM_MID, 300, -55);
    // 设置图片为可点击
    lv_obj_add_flag(music_next, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(music_next, music_btn_next_click_event_cb, LV_EVENT_CLICKED, NULL);
    // 显示进度条
    create_music_slider(music_screen);
    // 显示音量进度条
    create_music_volume_slider(music_screen);

    // 显示当前音乐进度
    cut_music_time_label = lv_label_create(music_screen);
    lv_label_set_text_fmt(cut_music_time_label, "%02d:%02d", 0, 0);
    lv_obj_align(cut_music_time_label, LV_ALIGN_BOTTOM_MID, -165, -24);
    lv_obj_set_style_text_font(cut_music_time_label, font_cut_music_time, 0);
    // 字体颜色
    lv_obj_set_style_text_color(cut_music_time_label, lv_color_hex(0xFFFFFF), 0);

    // 显示当前音乐总时长

    total_music_time_label = lv_label_create(music_screen);
    lv_label_set_text_fmt(total_music_time_label, "%02d:%02d", 0, 0);
    lv_obj_align(total_music_time_label, LV_ALIGN_BOTTOM_MID, 355, -24);
    lv_obj_set_style_text_font(total_music_time_label, font_cut_music_time, 0);
    // 字体颜色
    lv_obj_set_style_text_color(total_music_time_label, lv_color_hex(0xFFFFFF), 0);

    // 返回按钮
    lv_obj_t * back_btn = lv_btn_create(music_screen);
    lv_obj_set_size(back_btn, 80, 50);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 10, -15);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x87CEFA), 0);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_50, 0);
    lv_obj_t * back_btn_label = lv_label_create(back_btn);
    lv_label_set_text(back_btn_label, "返回");
    lv_obj_set_style_text_font(back_btn_label, font_music_list, 0);
    lv_obj_set_style_text_color(back_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_btn_label);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 切换视频按钮
    video_btn = lv_btn_create(music_screen);
    lv_obj_set_size(video_btn, 100, 50);
    lv_obj_align(video_btn, LV_ALIGN_BOTTOM_LEFT, 95, -15);
    lv_obj_set_style_bg_color(video_btn, lv_color_hex(0x87CEFA), 0);
    lv_obj_set_style_bg_opa(video_btn, LV_OPA_50, 0);
    lv_obj_t * video_btn_label = lv_label_create(video_btn);
    lv_label_set_text(video_btn_label, "切换视频");
    lv_obj_set_style_text_font(video_btn_label, font_music_list, 0);
    lv_obj_set_style_text_color(video_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(video_btn_label);
    lv_obj_add_event_cb(video_btn, video_convert_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 切换音乐按钮
    music_btn = lv_btn_create(music_screen);
    lv_obj_set_size(music_btn, 100, 50);
    lv_obj_align(music_btn, LV_ALIGN_BOTTOM_LEFT, 95, -15);
    lv_obj_set_style_bg_color(music_btn, lv_color_hex(0x87CEFA), 0);
    lv_obj_set_style_bg_opa(music_btn, LV_OPA_50, 0);
    lv_obj_t * music_btn_label = lv_label_create(music_btn);
    lv_label_set_text(music_btn_label, "切换音乐");
    lv_obj_set_style_text_font(music_btn_label, font_music_list, 0);
    lv_obj_set_style_text_color(music_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(music_btn_label);
    lv_obj_add_event_cb(music_btn, music_convert_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_add_flag(video_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(music_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(video_btn, LV_OBJ_FLAG_HIDDEN);

    // 创建一个垂直布局的容器
    lrc_container = lv_obj_create(music_screen);
    lv_obj_set_size(lrc_container, 260, 280); // 设置固定宽度，内容高度自适应
    //  lv_obj_set_style_local_pad_all(lrc_container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);

    lv_obj_align(lrc_container, LV_ALIGN_TOP_MID, 250, 55);
    lv_obj_set_flex_flow(lrc_container, LV_FLEX_FLOW_COLUMN); // 设置为竖向排列
    lv_obj_set_flex_align(lrc_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_START); // 居中对齐标签
                                                // 背景设置透明
    lv_obj_set_style_bg_opa(lrc_container, LV_OPA_TRANSP, 0);
    // 去除边框
    lv_obj_set_style_border_width(lrc_container, 0, 0);
    lv_obj_set_style_pad_all(lrc_container, 0, 0); // 移除所有的内边距
    lv_obj_remove_flag(lrc_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(lrc_container, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条
    lv_obj_set_style_pad_row(lrc_container, 15, 0);                  // 设置标签之间的垂直间距为 20 像素
    // 显示歌词
    music_lyric1 = lv_label_create(lrc_container);
    music_lyric2 = lv_label_create(lrc_container);
    music_lyric3 = lv_label_create(lrc_container);
    music_lyric4 = lv_label_create(lrc_container);
    music_lyric5 = lv_label_create(lrc_container);
    music_lyric6 = lv_label_create(lrc_container);
    music_lyric7 = lv_label_create(lrc_container);

    // 设置内容
    lv_label_set_text(music_lyric1, "");
    lv_label_set_text(music_lyric2, "");
    lv_label_set_text(music_lyric3, "");
    lv_label_set_text(music_lyric4, "");
    lv_label_set_text(music_lyric5, "");
    lv_label_set_text(music_lyric6, "");
    lv_label_set_text(music_lyric7, "");
    // TODO
    // 设置字体
    lv_obj_set_style_text_font(music_lyric1, font_music_lrc, 0);
    lv_obj_set_style_text_font(music_lyric2, font_music_lrc, 0);
    lv_obj_set_style_text_font(music_lyric3, font_music_lrc, 0);
    lv_obj_set_style_text_font(music_lyric4, font_music_lrc, 0);
    lv_obj_set_style_text_font(music_lyric5, font_music_lrc, 0);
    lv_obj_set_style_text_font(music_lyric6, font_music_lrc, 0);
    lv_obj_set_style_text_font(music_lyric7, font_music_lrc, 0);

    // 设置歌词颜色
    lv_obj_set_style_text_color(music_lyric1, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(music_lyric2, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(music_lyric3, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(music_lyric4, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(music_lyric5, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(music_lyric6, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(music_lyric7, lv_color_hex(0xFFFFFF), 0);

    // 设置字体宽度
    lv_obj_set_width(music_lyric1, 260);
    lv_obj_set_width(music_lyric2, 260);
    lv_obj_set_width(music_lyric3, 260);
    lv_obj_set_width(music_lyric4, 260);
    lv_obj_set_width(music_lyric5, 260);
    lv_obj_set_width(music_lyric6, 260);
    lv_obj_set_width(music_lyric7, 260);

    // 设置字体长模式
    lv_label_set_long_mode(music_lyric1, LV_LABEL_LONG_WRAP);
    lv_label_set_long_mode(music_lyric2, LV_LABEL_LONG_WRAP);
    lv_label_set_long_mode(music_lyric3, LV_LABEL_LONG_WRAP);
    lv_label_set_long_mode(music_lyric4, LV_LABEL_LONG_WRAP);
    lv_label_set_long_mode(music_lyric5, LV_LABEL_LONG_WRAP);
    lv_label_set_long_mode(music_lyric6, LV_LABEL_LONG_WRAP);
    lv_label_set_long_mode(music_lyric7, LV_LABEL_LONG_WRAP);

    // 设置字体透明度
    lv_obj_set_style_text_opa(music_lyric1, LV_OPA_50, 0);
    lv_obj_set_style_text_opa(music_lyric2, LV_OPA_50, 0);
    lv_obj_set_style_text_opa(music_lyric3, LV_OPA_50, 0);
    // lv_obj_set_style_text_opa(music_lyric1, LV_OPA_50, 0);
    lv_obj_set_style_text_opa(music_lyric5, LV_OPA_50, 0);
    lv_obj_set_style_text_opa(music_lyric6, LV_OPA_50, 0);
    lv_obj_set_style_text_opa(music_lyric7, LV_OPA_50, 0);
    // 加载信息
    if(music_state == 1 || music_state == 2) {
        load_music_screen_fun();
    }
}

/* Create a slider object */
// 滑块回调函数
static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int32_t value     = lv_slider_get_value(slider);
    /* Update the progress label or any other UI elements if needed */
    /* For example: lv_label_set_text_fmt(label, "%02d:%02d", min, sec); */
    if(music_state == 2) {
        music_state = 1;
    }
    if(video_state == 2) {
        video_state = 1;
    }
    lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
    int tar_time = (int)(round((value / 1000.0) * total_music_time));
    char cmd[128] = {0};
    sprintf(cmd, "echo seek %d 2 > /tmp/fifo", tar_time);
    system(cmd);
    cut_music_time = tar_time;
    if(music_state != 0) {
    jump_to_lyrics();
    }
}

void create_music_slider(lv_obj_t * parent)
{
    /* Create a slider */
    slider = lv_slider_create(parent);
    lv_obj_set_size(slider, 450, 5);
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 95, -30); // Align the slider to the center

    /* Set slider range and initial value */
    lv_slider_set_range(slider, 0, 1000);        // For example, 0 to 100% progress
    lv_slider_set_value(slider, 0, LV_ANIM_OFF); // Initial value

    /* Customize slider appearance */
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xE0F7FA), LV_PART_MAIN);      // Background color
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);                  // Set background opacity
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x42A5F5), LV_PART_INDICATOR); // Indicator color
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);             // Set indicator opacity

    /* Customize the knob */
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x8C9EFF), LV_PART_KNOB);     // Knob color
    lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);             // Make knob circular
    lv_obj_set_style_shadow_width(slider, 10, LV_PART_KNOB);                     // Add shadow to knob
    lv_obj_set_style_shadow_color(slider, lv_color_hex(0x42A5F5), LV_PART_KNOB); // Shadow color

    /* Add event callback */
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_CLICKED, NULL);
}
// 滑块回调函数
static void slider_volume_event_cb(lv_event_t * e)
{
    lv_obj_t * slider_volume = lv_event_get_target(e);
    music_volume             = lv_slider_get_value(volume_slider);
    /* Update the progress label or any other UI elements if needed */
    /* For example: lv_label_set_text_fmt(label, "%02d:%02d", min, sec); */
    printf("value:%d\n", music_volume);
    char cmd[128] = {0};
    sprintf(cmd, "echo volume %d 1 > /tmp/fifo", music_volume);
    system(cmd);
}

void create_music_volume_slider(lv_obj_t * parent)
{
    /* Create a slider */
    volume_slider = lv_slider_create(parent);
    lv_obj_set_size(volume_slider, 100, 5);
    lv_obj_align(volume_slider, LV_ALIGN_BOTTOM_MID, 205, -82); // Align the slider to the center

    /* Set slider range and initial value */
    lv_slider_set_range(volume_slider, 0, 100);                    // For example, 0 to 100% progress
    lv_slider_set_value(volume_slider, music_volume, LV_ANIM_OFF); // Initial value

    /* Customize slider appearance */
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0xE0F7FA), LV_PART_MAIN);      // Background color
    lv_obj_set_style_bg_opa(volume_slider, LV_OPA_COVER, LV_PART_MAIN);                  // Set background opacity
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x42A5F5), LV_PART_INDICATOR); // Indicator color
    lv_obj_set_style_bg_opa(volume_slider, LV_OPA_COVER, LV_PART_INDICATOR);             // Set indicator opacity

    /* Customize the knob */
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x8C9EFF), LV_PART_KNOB);     // Knob color
    lv_obj_set_style_radius(volume_slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);             // Make knob circular
    lv_obj_set_style_shadow_width(volume_slider, 10, LV_PART_KNOB);                     // Add shadow to knob
    lv_obj_set_style_shadow_color(volume_slider, lv_color_hex(0x42A5F5), LV_PART_KNOB); // Shadow color

    /* Add event callback */
    lv_obj_add_event_cb(volume_slider, slider_volume_event_cb, LV_EVENT_CLICKED, NULL);
}

void total_time_convert()
{
    total_minutes = (int)(total_music_time / 60);
    total_seconds = total_music_time - (total_minutes * 60);
}
void cut_time_convert()
{
    cut_minutes = (int)(cut_music_time / 60);
    cut_seconds = cut_music_time - (cut_minutes * 60);
}

void get_total_music_time()
{
    total_music_time = get_movie_duration();
    printf("total_music_time:%f\n", total_music_time);
    total_time_convert();
    char total_time_str[20] = {0};
    sprintf(total_time_str, "%02d:%02.0f", total_minutes, total_seconds);
    lv_label_set_text_fmt(total_music_time_label, "%s", total_time_str);
}
// 返回按钮回调函数
void * back_btn_event_cb(lv_event_t * e)
{
    // 获取音乐信息
    if(strlen(music_name_list) != 0) {
        char str[1024];
        strcpy(str, music_name_list[music_index]);
        char * name         = strtok(str, ".");
        char img_path[1024] = {0};
        sprintf(img_path, IMG_PATH "/%s.bmp", name);
        // 设置图片
        lv_img_set_src(home_music_img, img_path);

        if(strstr(name, "-") == NULL) {
            lv_label_set_text(home_music_name_label, name);
            lv_label_set_text(home_music_singer_label, "-未知");
        } else {
            char * music_singer_str = strtok(name, "-");
            lv_label_set_text_fmt(home_music_singer_label, "-%s", music_singer_str);

            char * music_name_str = strtok(NULL, "-");
            lv_label_set_text_fmt(home_music_name_label, "%s", music_name_str);
        }
    } else {
        printf("没有音乐\n");
    }
    if(music_state == 1) {
        lv_obj_add_flag(home_music_play, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(home_music_pause, LV_OBJ_FLAG_HIDDEN);

    } else if(music_state == 2) {
        lv_obj_add_flag(home_music_pause, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(home_music_play, LV_OBJ_FLAG_HIDDEN);
    }
    if(video_state != 0) {
        system("killall mplayer");
        video_state = 0;
        music_state = 0;
    }

    lv_obj_add_flag(music_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

// 重新加载页面
void load_music_screen_fun()
{
    char str[1024];
    strcpy(str, music_name_list[music_index]);
    char * name         = strtok(str, ".");
    char img_path[1024] = {0};
    sprintf(img_path, IMG_PATH "/%s.bmp", name);
    // 设置图片
    lv_img_set_src(music_img, img_path);
    // 获取子对象
    lv_obj_t * music_btn = lv_obj_get_child(music_list, music_index + 1);
    lv_obj_set_style_bg_color(music_btn, lv_color_hex(0x4B8BFF), 0);

    if(strstr(name, "-") == NULL) {
        lv_label_set_text(music_name, name);
        lv_label_set_text(music_singer, "-未知");
    } else {

        char * music_singer_str = strtok(name, "-");
        lv_label_set_text(music_singer, music_singer_str);
        lv_label_set_text_fmt(music_singer, "-%s", music_singer_str);

        char * music_name_str = strtok(NULL, "-");
        lv_label_set_text_fmt(music_name, "%s", music_name_str);
    }
    get_total_music_time();
    if(music_state == 1) {

    } else if(music_state == 2) {
        music_state = 1;
    }
    lv_obj_add_flag(music_play, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_pause, LV_OBJ_FLAG_HIDDEN);
    lv_slider_set_value(volume_slider, music_volume, LV_ANIM_OFF);
}

// 视频转换
void * video_convert_btn_event_cb(lv_event_t * e)
{

    lv_obj_add_flag(video_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(music_list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(video_list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(music_name, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(music_singer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(lrc_container, LV_OBJ_FLAG_HIDDEN);

    quit_movie();
    sleep(1);
    music_state = 0;
}

// 音乐转换
void * music_convert_btn_event_cb(lv_event_t * e)
{
    quit_movie(); // 退出视频
    usleep(100 * 1000);
    lv_obj_add_flag(music_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(video_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(video_list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_name, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(music_singer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(lrc_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(music_screen, lv_color_hex(0x181831), 0);
    video_state = 0;
    cut_line    = 4;
    total_line  = 0;
    memset(lrcfile, 0, sizeof(lrcfile));
}

// 获取歌词函数
void get_lyric()
{
    // 获取歌词
    char name[100];
    char lrc_path[1024];
    sscanf(music_name_list[music_index], "%[^.]", name);
    sprintf(lrc_path, LRC_PATH "/%s.lrc", name);
    printf("lrc_path:%s\n", lrc_path);
    FILE * lrcfp = fopen(lrc_path, "r"); // 打开歌词文件
    if(lrcfp == NULL) {
        printf("歌词文件打开失败\n");
        return;
    } else {
        printf("歌词文件打开成功\n");
    }
    cut_line   = 4;
    total_line = 0;
    memset(lrcfile, 0, sizeof(lrcfile));
    // 把歌词加载到数组中
    while(!feof(lrcfp)) {
        fgets(lrcfile[total_line], sizeof(lrcfile[total_line]), lrcfp);
        total_line++;
    }
    fclose(lrcfp);
    // 设置几句到标签中
    char str[1024] = {0};
    strcpy(str, lrcfile[0]);
    char * start = strchr(str, ']');
    start++;
    start[strlen(start) - 1] = '\0';
    lv_label_set_text(music_lyric1, start);

    strcpy(str, lrcfile[1]);
    start = strchr(str, ']');
    start++;
    start[strlen(start) - 1] = '\0';
    lv_label_set_text(music_lyric2, start);

    strcpy(str, lrcfile[2]);
    start = strchr(str, ']');
    start++;
    start[strlen(start) - 1] = '\0';
    lv_label_set_text(music_lyric3, start);

    strcpy(str, lrcfile[3]);
    start = strchr(str, ']');
    start++;
    start[strlen(start) - 1] = '\0';
    lv_label_set_text(music_lyric4, start);

    strcpy(str, lrcfile[4]);
    start = strchr(str, ']');
    start++;
    start[strlen(start) - 1] = '\0';
    lv_label_set_text(music_lyric5, start);

    strcpy(str, lrcfile[5]);
    start = strchr(str, ']');
    start++;
    start[strlen(start) - 1] = '\0';
    lv_label_set_text(music_lyric6, start);

    strcpy(str, lrcfile[6]);
    start = strchr(str, ']');
    start++;
    start[strlen(start) - 1] = '\0';
    lv_label_set_text(music_lyric7, start);
}