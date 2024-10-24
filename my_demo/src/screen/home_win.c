#include "../inc/home_win.h"
#include "../inc/TCP_client.h"
#include "../inc/thread_pool.h"
#include "../inc/TCP_get_weather.h"
#include "../inc/music_win.h"
#include "../inc/address_book_win.h"
#include "../inc/map_win.h"

lv_obj_t * root;                  // 主页面
lv_obj_t * bg_img;                // 背景图片
lv_obj_t * time_label;            // 左上角时间
lv_obj_t * speed_label;           // 速度
lv_obj_t * mileage_label_val;     // 里程
lv_obj_t * electricity_label_val; // 电量

lv_obj_t * home_music_name_label;
lv_obj_t * home_music_singer_label;
lv_obj_t * home_music_img;

lv_obj_t * home_music_prev;
lv_obj_t * home_music_play;
lv_obj_t * home_music_pause;
lv_obj_t * home_music_next;

extern pthread_mutex_t lock;

void * tcp_client_task(void * arg)
{ 
    tcp_client_init();
    
}

void * get_time_task(void * arg)
{
    while(1) {

        time_t current_time;
        struct tm * time_info;
        char time_string[100]; // 存储格式化的时间字符串

        // 获取当前时间
        time(&current_time);

        // 转换为本地时间
        time_info = localtime(&current_time);

        // 格式化时间（自定义格式）
        strftime(time_string, 100, "%H:%M", time_info);

        // 上锁

        pthread_mutex_lock(&lock);
        lv_label_set_text(time_label, time_string); // 更新标签的内容
        pthread_mutex_unlock(&lock);

        // 每秒更新一次
        sleep(1);
    }
    return NULL;
}
// 图片点击回调函数
void * image_click_event_cb(lv_event_t * e)
{
    lv_obj_t * img = lv_event_get_target(e);
    lv_point_t point;
    // 获取点击位置的坐标
    lv_indev_get_point(lv_indev_get_act(), &point);
    printf("Clicked at (%d, %d)\n", point.x, point.y);
    // 判断点击是否在图片的特定区域内，例如在 (20, 20) 到 (50, 50)
    if(point.x >= 13 && point.x <= 70 && point.y >= 135 && point.y <= 198) {
        printf("进入地图\n");
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);           // 隐藏主页面
        map_win_init();                                    // 初始化地图页面
    } else if(point.x >= 13 && point.x <= 70 && point.y >= 211 && point.y <= 271) {
        printf("进入音乐播放器\n");
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);           // 隐藏主页面
        music_win_init();                                    // 初始化音乐播放器页面
        lv_obj_clear_flag(music_screen, LV_OBJ_FLAG_HIDDEN); // 显示音乐播放器页面
    } else if(point.x >= 13 && point.x <= 70 && point.y >= 284 && point.y <= 345) {
        printf("进入电话溥\n");
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);                  // 隐藏主页面
        address_book_win_init();                                    // 初始化通讯录页面
        lv_obj_clear_flag(address_book_screen, LV_OBJ_FLAG_HIDDEN); // 显示通讯录页面
    }
}

void home_win_init()
{
    lv_obj_t * font_time = lv_freetype_font_create(TTF_PATH "/simkai.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 20,
                                                   LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_time) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    lv_obj_t * font_speed = lv_freetype_font_create(TTF_PATH "/simkai.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 100,
                                                    LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_speed) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    lv_obj_t * font_mileage = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 25, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_mileage) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    lv_obj_t * font_mileage_val = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 40, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_mileage_val) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    lv_obj_t * font_mileage_val_unit = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 15, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_mileage_val_unit) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    lv_obj_t * font_area = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 20, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_area) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    lv_obj_t * font_music_name = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 25, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_area) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }


    // 1、初始化线程池
    thread_pool_p pool = malloc(sizeof(thread_pool));
    THREAD_POOL_Init(pool, 2);

    // 创建根页面对象
    root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root, 800, 480);
    lv_obj_set_style_pad_all(root, 0, 0); // 移除所有的内边距
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条

    // 创建背景图片
    bg_img = lv_img_create(root);
    lv_img_set_src(bg_img, IMG_PATH "/home_win_bg.bmp");
    lv_obj_center(bg_img); // 居中显示背景图片

    // 创建图片点击回调函数
    // 设置图片为可点击
    lv_obj_add_flag(bg_img, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(bg_img, image_click_event_cb, LV_EVENT_CLICKED, NULL);

    // 显示左上角时间
    time_label = lv_label_create(root);
    lv_label_set_text(time_label, "00:00");
    lv_obj_set_style_text_font(time_label, font_time, 0);
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 14, 17);
    // 设置文本对齐方式
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(time_label, 50);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);
    // 设置线程更新时间

    // 显示车速
    speed_label = lv_label_create(root);
    lv_label_set_text(speed_label, "48");
    lv_obj_set_style_text_font(speed_label, font_speed, 0);
    lv_obj_align(speed_label, LV_ALIGN_CENTER, -142, -105);
    lv_obj_set_style_text_color(speed_label, lv_color_hex(0xFFFFFF), 0);

    // 显示里程
    lv_obj_t * mileage_label = lv_label_create(root);
    lv_label_set_text(mileage_label, "总里程");
    lv_obj_set_style_text_font(mileage_label, font_mileage, 0);
    lv_obj_align(mileage_label, LV_ALIGN_TOP_MID, 170, 85);
    lv_obj_set_style_text_color(mileage_label, lv_color_hex(0xFFFFFF), 0);

    // 创建水平容器，将数值和单位放在同一个容器中
    lv_obj_t * mileage_container = lv_obj_create(root);
    lv_obj_clear_flag(mileage_container, LV_OBJ_FLAG_SCROLLABLE); // 不需要滚动
    lv_obj_set_flex_flow(mileage_container, LV_FLEX_ALIGN_START); // 使用水平布局
    lv_obj_align(mileage_container, LV_ALIGN_TOP_MID, 230, 110);  // 设置容器的位置
    lv_obj_set_style_pad_column(mileage_container, 7, 0);         // 去除容器内元素之间的列间距
    lv_obj_set_style_pad_all(mileage_container, 0, 0);            // 去除容器内的所有内边距
    lv_obj_set_style_bg_opa(mileage_container, LV_OPA_TRANSP, 0); // 容器透明背景
    // 将容器边框变透明
    lv_obj_set_style_border_opa(mileage_container, LV_OPA_TRANSP, 0);
    // 容器自动根据内容宽度调整大小
    lv_obj_set_width(mileage_container, 200);

    // 数值标签
    mileage_label_val = lv_label_create(mileage_container);
    lv_label_set_text(mileage_label_val, "8888");
    lv_obj_set_style_text_font(mileage_label_val, font_mileage_val, 0);
    lv_obj_set_style_text_align(mileage_label_val, LV_TEXT_ALIGN_LEFT, 0); // 设置文本左对齐
    lv_obj_set_style_text_color(mileage_label_val, lv_color_hex(0xFFFFFF), 0);

    // 单位标签
    lv_obj_t * mileage_label_val_unit = lv_label_create(mileage_container);
    lv_label_set_text(mileage_label_val_unit, "km");
    lv_obj_set_style_text_font(mileage_label_val_unit, font_mileage_val_unit, 0);
    lv_obj_set_style_text_color(mileage_label_val_unit, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_translate_y(mileage_label_val_unit, 23, 0); // 设置单位标签向下偏移5个像素

    // 创建水平容器，将数值和单位放在同一个容器中
    lv_obj_t * electricity_container = lv_obj_create(root);
    lv_obj_clear_flag(electricity_container, LV_OBJ_FLAG_SCROLLABLE); // 不需要滚动
    lv_obj_set_flex_flow(electricity_container, LV_FLEX_ALIGN_START); // 使用水平布局
    lv_obj_align(electricity_container, LV_ALIGN_TOP_MID, 230, 180);  // 设置容器的位置
    lv_obj_set_style_pad_column(electricity_container, 7, 0);         // 去除容器内元素之间的列间距
    lv_obj_set_style_pad_all(electricity_container, 0, 0);            // 去除容器内的所有内边距
    lv_obj_set_style_bg_opa(electricity_container, LV_OPA_TRANSP, 0); // 容器透明背景
    // 将容器边框变透明
    lv_obj_set_style_border_opa(electricity_container, LV_OPA_TRANSP, 0);
    // 容器自动根据内容宽度调整大小
    lv_obj_set_width(electricity_container, 200);

    // 显示电量
    lv_obj_t * electricity_label = lv_label_create(root);
    lv_label_set_text(electricity_label, "电量");
    lv_obj_set_style_text_font(electricity_label, font_mileage, 0);
    lv_obj_align(electricity_label, LV_ALIGN_TOP_MID, 158, 180);
    lv_obj_set_style_text_color(electricity_label, lv_color_hex(0xFFFFFF), 0);

    // 显示电量数值
    electricity_label_val = lv_label_create(root);
    lv_label_set_text(electricity_label_val, "68%");
    lv_obj_set_style_text_font(electricity_label_val, font_mileage_val, 0);
    lv_obj_align(electricity_label_val, LV_ALIGN_TOP_MID, 233, 205);
    lv_obj_set_width(electricity_label_val, 200);
    lv_obj_set_style_text_align(electricity_label_val, LV_TEXT_ALIGN_LEFT, 0); // 设置文本左对齐
    lv_obj_set_style_text_color(electricity_label_val, lv_color_hex(0xFFFFFF), 0);

    // 设置地区
    char * area = "玉林市";
    // 显示地区
    lv_obj_t * area_label = lv_label_create(root);
    lv_label_set_text(area_label, area);
    lv_obj_set_style_text_font(area_label, font_area, 0);
    lv_obj_align(area_label, LV_ALIGN_BOTTOM_LEFT, 125, -120);
    lv_obj_set_style_text_color(area_label, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t * date_label;
    lv_obj_t * temperature_label;
    lv_obj_t * weather_label;
    lv_obj_t * manner_label;
    lv_obj_t * pm_label;
    lv_obj_t * img;

    // 显示时间
    date_label = lv_label_create(root);
    lv_label_set_text(date_label, "2014/01/01  星期四");
    lv_obj_set_style_text_font(date_label, font_area, 0);
    lv_obj_align(date_label, LV_ALIGN_BOTTOM_LEFT, 230, -120);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xFFFFFF), 0);

    // 显示今日温度
    temperature_label = lv_label_create(root);
    lv_label_set_text(temperature_label, "38℃");
    lv_obj_set_style_text_font(temperature_label, font_mileage_val, 0);
    lv_obj_align(temperature_label, LV_ALIGN_BOTTOM_LEFT, 125, -60);
    lv_obj_set_style_text_color(temperature_label, lv_color_hex(0xFFFFFF), 0);

    // 显示今日天气
    weather_label = lv_label_create(root);
    lv_label_set_text(weather_label, "下雨");
    lv_obj_set_style_text_font(weather_label, font_area, 0);
    lv_obj_align(weather_label, LV_ALIGN_BOTTOM_LEFT, 125, -40);
    lv_obj_set_style_text_color(weather_label, lv_color_hex(0xFFFFFF), 0);

    // 显示今日风向
    manner_label = lv_label_create(root);
    lv_label_set_text(manner_label, "南风");
    lv_obj_set_style_text_font(manner_label, font_area, 0);
    lv_obj_align(manner_label, LV_ALIGN_BOTTOM_LEFT, 230, -75);
    lv_obj_set_style_text_color(manner_label, lv_color_hex(0xFFFFFF), 0);

    // 显示今日风级
    pm_label = lv_label_create(root);
    lv_label_set_text(pm_label, "风级:1-3");
    lv_obj_set_style_text_font(pm_label, font_area, 0);
    lv_obj_align(pm_label, LV_ALIGN_BOTTOM_LEFT, 230, -40);
    lv_obj_set_style_text_color(pm_label, lv_color_hex(0xFFFFFF), 0);

    // 显示天气图标
    img = lv_img_create(root);
    lv_img_set_src(img, IMG_PATH "/weather_sunny.png");
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 330, -30);

    // 获取天气信息
    struct weather_list * weather_list = malloc(sizeof(struct weather_list));

    weather_list = tcp_get_weather(area);
    if(weather_list != NULL) {
        lv_label_set_text_fmt(date_label, "%s   %s", weather_list->date, weather_list->week);
        // char * tem = strtok(weather_list->data[1].temperature, "～");
        lv_label_set_text_fmt(temperature_label, "%s℃", weather_list->temp);
        lv_label_set_text_fmt(weather_label, "%s", weather_list->weather);
        lv_label_set_text_fmt(manner_label, "%s风", weather_list->wind);
        lv_label_set_text_fmt(pm_label, "风级:%s", weather_list->power);
        if(strcmp(weather_list->weather, "晴") == 0) {
            lv_img_set_src(img, IMG_PATH "/weather_sunny.png");
        } else if(strcmp(weather_list->weather, "多云") == 0) {
            lv_img_set_src(img, IMG_PATH "/weather_cloudy.png");
        } else if(strstr(weather_list->weather, "阴") != NULL || (strstr(weather_list->weather, "雾") != NULL)) {
            lv_img_set_src(img, IMG_PATH "/weather_cloudy.png");
        } else if(strstr(weather_list->weather, "雨") != NULL) {
            lv_img_set_src(img, IMG_PATH "/weather_rain.png");
        }
    }

    // 音乐名称
    home_music_name_label = lv_label_create(root);
    lv_label_set_text(home_music_name_label, "-----");
    lv_obj_align(home_music_name_label, LV_ALIGN_BOTTOM_MID, 275, -108);
    lv_obj_set_style_text_font(home_music_name_label, font_music_name, 0);
    lv_obj_set_style_text_color(home_music_name_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_long_mode(home_music_name_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(home_music_name_label, 180);

    // 歌手名称
    home_music_singer_label = lv_label_create(root);
    lv_label_set_text(home_music_singer_label, "--");
    lv_obj_align(home_music_singer_label, LV_ALIGN_BOTTOM_MID, 277, -88);
    lv_obj_set_style_text_font(home_music_singer_label, font_mileage_val_unit, 0);
    lv_obj_set_style_text_color(home_music_singer_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_long_mode(home_music_singer_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(home_music_singer_label, 180);

    // 音乐图片
    static lv_style_t style;
    lv_style_init(&style);
    /*Add a shadow*/

    lv_style_set_shadow_width(&style, 5);
    lv_style_set_shadow_color(&style, lv_color_black());
    lv_obj_t * container = lv_obj_create(root);
    lv_obj_set_size(container, 108, 108);                    // 设置容器大小为正方形
    lv_obj_set_style_radius(container, LV_RADIUS_CIRCLE, 0); // 设置圆角为圆形
    lv_obj_set_style_clip_corner(container, true, 0);       // 启用裁剪，确保超出容器区域的内容被剪掉
    lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 115, -33); // 使容器居中或根据需要设置位置
    lv_obj_set_style_pad_all(container, 0, 0);              // 移除所有的内边距
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条

    lv_obj_add_style(container, &style, 0); 

    home_music_img = lv_img_create(container);
    lv_img_set_src(home_music_img, IMG_PATH "/music_bg.bmp");
    lv_obj_align(home_music_img, LV_ALIGN_CENTER, 0, 0); // 将图片居中对齐
    //  根据需要调整图片大小
    lv_obj_set_size(home_music_img, 108, 108); // 使图片大小与容器匹配，保持为正方形
    // lv_img_set_zoom(home_music_img, 128);

    // 显示上一首按钮
    home_music_prev = lv_img_create(root);
    lv_img_set_src(home_music_prev, IMG_PATH "/btn_prev.png");
    lv_obj_align(home_music_prev, LV_ALIGN_BOTTOM_MID, 200, -22);
    // 设置图片为可点击
    lv_obj_add_flag(home_music_prev, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(home_music_prev, music_btn_prev_click_event_cb, LV_EVENT_CLICKED, NULL);

    // 播放按钮
    home_music_play = lv_img_create(root);
    lv_img_set_src(home_music_play, IMG_PATH "/btn_play.png");
    lv_obj_align(home_music_play, LV_ALIGN_BOTTOM_MID, 270, -15);
    // 设置图片为可点击
    lv_obj_add_flag(home_music_play, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(home_music_play, music_btn_play_click_event_cb, LV_EVENT_CLICKED, NULL);

    // 暂停按钮
    home_music_pause = lv_img_create(root);
    lv_img_set_src(home_music_pause, IMG_PATH "/btn_pause.png");
    lv_obj_align(home_music_pause, LV_ALIGN_BOTTOM_MID, 270, -15);
    // 设置图片为可点击
    lv_obj_add_flag(home_music_pause, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(home_music_pause, music_btn_pause_click_event_cb, LV_EVENT_CLICKED, NULL);

    // 添加隐藏属性
    lv_obj_add_flag(home_music_play, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(home_music_pause, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(home_music_play, LV_OBJ_FLAG_HIDDEN);

    // 显示下一首按钮
    home_music_next = lv_img_create(root);
    lv_img_set_src(home_music_next, IMG_PATH "/btn_next.png");
    lv_obj_align(home_music_next, LV_ALIGN_BOTTOM_MID, 340, -22);
    // 设置图片为可点击
    lv_obj_add_flag(home_music_next, LV_OBJ_FLAG_CLICKABLE);
    // 添加事件回调函数
    lv_obj_add_event_cb(home_music_next, music_btn_next_click_event_cb, LV_EVENT_CLICKED, NULL);

    THREAD_POOL_AddTask(pool, get_time_task, NULL);
    THREAD_POOL_AddTask(pool, tcp_client_task, NULL);
}