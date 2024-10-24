#include "../inc/map_win.h"
#include "../inc/home_win.h"
lv_obj_t * map_screen;
lv_obj_t * label_font;
lv_obj_t * textBox_font;
static lv_obj_t * e_input_box;
static lv_obj_t * w_input_box;
static lv_obj_t * map_img;
void keyboard_event_handler(lv_event_t * e)
{
    lv_obj_t * kb = lv_event_get_target(e);
    // 处理键盘事件，例如获取输入
}
// 输入框回调函数
static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta        = lv_event_get_target(e);
    lv_obj_t * kb        = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
    }
}

// 返回按钮回调函数
static void back_btn_event_cb()
{
    lv_obj_add_flag(map_screen, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(map_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

// 查询回调函数
static void search_btn_event_cb()
{
    // 获取输入框的值
    const char * e_text = lv_textarea_get_text(e_input_box);
    const char * w_text = lv_textarea_get_text(w_input_box);
    printf("e_text:%s,w_text:%s\n", e_text, w_text);
    get_map(strtod(e_text, NULL), strtod(w_text,NULL), 550, 480, 10);

    
}

void map_win_init()
{
    label_font = lv_freetype_font_create(TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 25,
                                         LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!label_font) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    textBox_font = lv_freetype_font_create(TTF_PATH "/DingTalk-JinBuTi.ttf", LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 20,
                                           LV_FREETYPE_FONT_STYLE_NORMAL);
    if(!label_font) {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    map_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(map_screen, 800, 480);
    lv_obj_set_style_pad_all(map_screen, 0, 0); // 移除所有的内边距
    lv_obj_remove_flag(map_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(map_screen, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条
    // 设置背景颜色
    lv_obj_t * bg_scrren = lv_obj_create(map_screen);
    lv_obj_set_size(bg_scrren, 250, 480);
    lv_obj_set_style_bg_color(bg_scrren, lv_color_hex(0xF6F6F6), 0);
    lv_obj_remove_flag(bg_scrren, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(bg_scrren, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条

    // 返回按钮
    lv_obj_t * back_btn = lv_btn_create(map_screen);
    lv_obj_set_size(back_btn, 100, 50);
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t * back_btn_label = lv_label_create(back_btn);
    lv_label_set_text(back_btn_label, "返回");
    lv_obj_set_style_text_font(back_btn_label, textBox_font, 0);
    lv_obj_set_style_text_color(back_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_btn_label);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 查询按钮
    lv_obj_t * search_btn = lv_btn_create(map_screen);
    lv_obj_set_size(search_btn, 100, 50);
    lv_obj_align(search_btn, LV_ALIGN_LEFT_MID, 135, 0);
    lv_obj_t * search_btn_label = lv_label_create(search_btn);
    lv_label_set_text(search_btn_label, "查询");
    lv_obj_set_style_text_font(search_btn_label, textBox_font, 0);
    lv_obj_set_style_text_color(search_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(search_btn_label);
    lv_obj_add_event_cb(search_btn, search_btn_event_cb, LV_EVENT_CLICKED, NULL);

     

    map_img = lv_image_create(lv_screen_active());
    lv_image_set_src(map_img, IMG_PATH"/map.png");
    lv_obj_align(map_img, LV_ALIGN_TOP_RIGHT, 0, 0);
    create_num_keyboard(map_screen);
}

// http://api.map.baidu.com/staticimage/v2?ak=CYtEjAAlJDE8BNkWAhwFkxJ8vZzUdGxa&center=113E,23W&width=800&height=600&zoom=10
// 获取地图函数
int get_map(double E, double W, int width, int height, int zoom)
{
    // 创建客户局对象
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 设置HTTP 服务器IP地址与端口 14.215.182.24:80
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(80);                  // http服务器默认是 80
    addr.sin_addr.s_addr = inet_addr("14.215.182.24"); // 自己找到对应的IP

    // 链接http 服务器端
    int ret = connect(tcp_socket, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        perror("链接服务器失败\n");
        return -1;
    } else {
        printf("链接http服务器成功\n");
    }

    // 发起http 请求
    // http://api.map.baidu.com/staticimage/v2?ak=CYtEjAAlJDE8BNkWAhwFkxJ8vZzUdGxa&center=113E,23W&width=800&height=600&zoom=10
    // char *http_Request = "GET /api.php?key=free&appid=0&msg=你好 HTTP/1.1\r\nHost:api.qingyunke.com\r\n\r\n";
    // char *http_Request = "GET
    // /staticimage/v2?ak=CYtEjAAlJDE8BNkWAhwFkxJ8vZzUdGxa&center=113E,23W&width=800&height=600&zoom=10
    // HTTP/1.1\r\nHost:api.map.baidu.com\r\n\r\n";

    char http_Request[4096] = {0};
    sprintf(http_Request,
            "GET /staticimage/v2?ak=CYtEjAAlJDE8BNkWAhwFkxJ8vZzUdGxa&center=%fE,%fW&width=%d&height=%d&zoom=%d "
            "HTTP/1.1\r\nHost:api.map.baidu.com\r\n\r\n",
            E, W, width, height, zoom);

    // 请求协议返回jpeg
    write(tcp_socket, http_Request, strlen(http_Request));

    // system("mkdir /home/gec/ -p");
    const char * prefix = "A:";                                   // 要去掉的前缀
    char extracted_path[256];                                     // 存储提取后的路径
    strcpy(extracted_path, IMG_PATH "/map.png" + strlen(prefix)); // 去掉前缀
    // 新建图片文件
    int fd = open(extracted_path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if(fd < 0) {
        perror("新建文件失败\n");
        return -1;
    }

    // 读取头http头数据
    char head[4096] = {0};
    int size        = read(tcp_socket, head, 4096);

    // 获取文件大小
    char * p = strstr(head, "Content-Length:"); // 指向最后的\r
    p += 16;                                    // 指向数据部分
    int file_size = atoi(p);
    printf("文件大小 %d\n", file_size);

    // 去掉头信息
    p = strstr(head, "\r\n\r\n"); // 指向最后的\r
    p += 4;                       // 指向数据部分

    // 去头后的数据
    write(fd, p, size - (p - head));

    int down_size = size - (p - head); // 记录第一次下载大小

    while(1) {
        // 读取响应数据
        char buf[4096] = {0};
        int size       = read(tcp_socket, buf, 4096);
        // 写入本地文件
        write(fd, buf, size);
        down_size += size;
        if(down_size >= file_size) {
            break;
        }
    }
    lv_image_set_src(map_img, IMG_PATH"/map.png");
    // 关闭通信
    close(tcp_socket);
}

void create_num_keyboard(lv_obj_t * parent)
{

    // 创建键盘对象
    lv_obj_t * keyboard = lv_keyboard_create(parent);
    lv_keyboard_set_textarea(keyboard, e_input_box); // 绑定输入框

    // 自定义按键布局

    static const char * kb_map[] = {
        "1", "2", "3", "\n", "4", "5", "6", "\n", "7", "8", "9", "\n", ".", "0", LV_SYMBOL_BACKSPACE, "\n"};

    /*Set the relative width of the buttons and other controls*/
    static const lv_buttonmatrix_ctrl_t kb_ctrl[] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

    // 设置自定义键盘布局
    lv_keyboard_set_map(keyboard, LV_KEYBOARD_MODE_USER_1, kb_map, kb_ctrl);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_USER_1);

    // 设置键盘位置
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_pos(keyboard, 0, 50);     // 调整位置
    lv_obj_set_size(keyboard, 250, 250); // 设置键盘大小

    // 事件处理
    lv_obj_add_event_cb(keyboard, keyboard_event_handler, LV_EVENT_ALL, NULL);

    // 显示提示词
    lv_obj_t * e_label = lv_label_create(parent);
    lv_label_set_text(e_label, "经度:");
    lv_obj_set_pos(e_label, 10, 20);
    lv_obj_set_style_text_font(e_label, label_font, 0);
    // 创建输入框
    e_input_box = lv_textarea_create(parent);
    lv_obj_set_size(e_input_box, 160, 50);
    lv_obj_set_pos(e_input_box, 75, 10);
    lv_obj_set_style_text_font(e_input_box, textBox_font, 0);
    lv_obj_add_event_cb(e_input_box, ta_event_cb, LV_EVENT_ALL, keyboard);

    // 显示提示词
    lv_obj_t * w_label = lv_label_create(parent);
    lv_label_set_text(w_label, "纬度:");
    lv_obj_set_pos(w_label, 10, 80);
    lv_obj_set_style_text_font(w_label, label_font, 0);
    // 创建输入框
    w_input_box = lv_textarea_create(parent);
    lv_obj_set_size(w_input_box, 160, 50);
    lv_obj_set_pos(w_input_box, 75, 70);
    lv_obj_set_style_text_font(w_input_box, textBox_font, 0);
    lv_obj_add_event_cb(w_input_box, ta_event_cb, LV_EVENT_ALL, keyboard);
}