#include "../inc/address_book_win.h"
#include "../inc/home_win.h"
#include "../inc/thread_pool.h"

static void *back_btn_event_cb(lv_event_t * e);

lv_obj_t * address_book_screen = NULL;
lv_obj_t * contacts_list       = NULL;

static thread_pool_p pool = NULL; // 线程池

lv_obj_t * input_number_label; // 输入号码的标签
char number[20]  = {0};        // 存储输入的号码
int number_index = 0;          // 记录输入号码的长度


void generate_random_phone_number(char * phone_number)
{
    // 生成随机的电话号码
    phone_number[0] = '1';              // 第一个数字固定为 1
    phone_number[1] = '3' + rand() % 7; // 第二个数字可以是 3 到 9 中的任意一个
    for(int i = 2; i < 11; i++) {
        phone_number[i] = '0' + rand() % 10; // 后面的数字随机生成 0 到 9
    }
    phone_number[11] = '\0'; // 结束字符串
}

void * address_book_bg_event_cb(lv_event_t * e)
{
    lv_obj_t * img = lv_event_get_target(e);
    lv_point_t point;
    // 获取点击位置的坐标
    lv_indev_get_point(lv_indev_get_act(), &point);
    printf("Clicked at (%d, %d)\n", point.x, point.y);
    // 判断点击是否在图片的特定区域内，例如在 (20, 20) 到 (50, 50)
    if(point.x >= 346 && point.x <= 467 && point.y >= 74 && point.y <= 160) {
        if(number_index < 11) {
            printf("1\n");
            number[number_index] = '1';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 487 && point.x <= 629 && point.y >= 74 && point.y <= 160) {
        if(number_index < 11) {
            printf("2\n");
            number[number_index] = '2';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 642 && point.x <= 764 && point.y >= 74 && point.y <= 160) {
        if(number_index < 11) {
            printf("3\n");
            number[number_index] = '3';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 346 && point.x <= 467 && point.y >= 164 && point.y <= 239) {
        if(number_index < 11) {
            printf("4\n");
            number[number_index] = '4';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 487 && point.x <= 629 && point.y >= 164 && point.y <= 239) {
        if(number_index < 11) {
            printf("5\n");
            number[number_index] = '5';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 642 && point.x <= 764 && point.y >= 164 && point.y <= 239) {
        if(number_index < 11) {
            printf("6\n");
            number[number_index] = '6';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 346 && point.x <= 467 && point.y >= 246 && point.y <= 335) {
        if(number_index < 11) {
            printf("7\n");
            number[number_index] = '7';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 487 && point.x <= 629 && point.y >= 246 && point.y <= 335) {
        if(number_index < 11) {
            printf("8\n");
            number[number_index] = '8';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 642 && point.x <= 764 && point.y >= 246 && point.y <= 335) {
        if(number_index < 11) {
            printf("9\n");
            number[number_index] = '9';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }

    } else if(point.x >= 346 && point.x <= 467 && point.y >= 338 && point.y <= 411) {
        if(number_index < 11) {
            printf("*\n");
            number[number_index] = '*';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 487 && point.x <= 629 && point.y >= 338 && point.y <= 406) {
        if(number_index < 11) {
            printf("0\n");
            number[number_index] = '0';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 642 && point.x <= 764 && point.y >= 338 && point.y <= 411) {
        if(number_index < 11) {
            printf("#\n");
            number[number_index] = '#';
            number_index++;
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    } else if(point.x >= 750 && point.x <= 800 && point.y >= 0 && point.y <= 51) {

        if(number_index > 0) {
            printf("退格\n");
            number_index--;
            number[number_index] = '\0';
            lv_label_set_text_fmt(input_number_label, "%s", number);
        }
    }
}

void address_book_win_init(void)
{
    lv_obj_t * font_contacts_name = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 50, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_contacts_name) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    lv_obj_t * font_contacts_list = lv_freetype_font_create(
        TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 20, LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!font_contacts_list) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }
    // 初始化线程池
    pool = malloc(sizeof(thread_pool));
    THREAD_POOL_Init(pool, 2);

    address_book_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(address_book_screen, 800, 480);
    lv_obj_set_style_pad_all(address_book_screen, 0, 0); // 移除所有的内边距
    lv_obj_remove_flag(address_book_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(address_book_screen, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条

    // 设置背景图片
    lv_obj_t * address_book_bg_img = lv_img_create(address_book_screen);
    lv_img_set_src(address_book_bg_img, IMG_PATH "/address_book_bg.bmp");
    lv_obj_align(address_book_bg_img, LV_ALIGN_CENTER, 0, 0);
    // 设置图片可点击
    lv_obj_add_flag(address_book_bg_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(address_book_bg_img, address_book_bg_event_cb, LV_EVENT_CLICKED, NULL);

    // 输入号码标签
    input_number_label = lv_label_create(address_book_screen);

    lv_label_set_text(input_number_label, "");
    lv_obj_align(input_number_label, LV_ALIGN_TOP_MID, 145, 0);
    lv_obj_set_style_text_font(input_number_label, font_contacts_name, 0);
    // 设置字体为白色
    lv_obj_set_style_text_color(input_number_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_width(input_number_label, 400);
    // 设置字体居中
    lv_obj_set_style_text_align(input_number_label, LV_TEXT_ALIGN_CENTER, 0);

    // 联系人列表
    contacts_list = lv_list_create(address_book_screen);

    lv_obj_set_size(contacts_list, 315, 400);
    lv_obj_align(contacts_list, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_pad_all(contacts_list, 5, 0); // 移除所有的内边距
    // 设置列表背景透明
    lv_obj_set_style_bg_opa(contacts_list, LV_OPA_TRANSP, 0);
    // 去除边框
    lv_obj_set_style_border_width(contacts_list, 0, 0);

    srand(time(NULL));     // 设置随机种子
    for(int i = 0; i < 10; i++) {
        
        char phone_number[12]; // 存储电话号码（11 位 + 结束符）

        // 生成并打印随机电话号码
        generate_random_phone_number(phone_number);
        lv_obj_t * list_btn = lv_list_add_btn(contacts_list, LV_SYMBOL_CALL, phone_number);
        lv_obj_set_size(list_btn, 300, 60);
        lv_obj_set_style_bg_opa(list_btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(list_btn, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(list_btn, &lv_font_montserrat_34, 0);
    }

    //返回按钮
    
    lv_obj_t * back_btn = lv_btn_create(address_book_screen);
    lv_obj_set_size(back_btn, 80, 50);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 10, -15);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x87CEFA), 0);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_50, 0);
    lv_obj_t * back_btn_label = lv_label_create(back_btn);
    lv_label_set_text(back_btn_label, "返回");
    lv_obj_set_style_text_font(back_btn_label, font_contacts_list, 0);
    lv_obj_set_style_text_color(back_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_btn_label);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    //添加线程任务
    // THREAD_POOL_AddTask(pool, search_number_task, NULL);
}

void *back_btn_event_cb(lv_event_t * e){
     lv_obj_add_flag(address_book_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}