#include "../../../inc/TCP_get_weather.h"
#include "../../../inc/TCP_client.h"

// 将数字星期转换为中文星期
const char * convert_week_to_chinese(const char * week)
{
    if(strcmp(week, "1") == 0) return "星期一";
    if(strcmp(week, "2") == 0) return "星期二";
    if(strcmp(week, "3") == 0) return "星期三";
    if(strcmp(week, "4") == 0) return "星期四";
    if(strcmp(week, "5") == 0) return "星期五";
    if(strcmp(week, "6") == 0) return "星期六";
    if(strcmp(week, "7") == 0) return "星期天";
    return "未知";
}

struct weather_list * tcp_get_weather(char * city)
{
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_socket < 0) {
        perror("创建套接字失败\n");
        return NULL;
    }

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(80);
    addr.sin_addr.s_addr = inet_addr("120.77.134.169");

    int ret = connect(tcp_socket, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        perror("链接服务器失败\n");
        close(tcp_socket);
        return NULL;
    } else {
        printf("链接服务器成功\n");
    }

    // 构造 HTTP 请求
    char http_Request[1024] = {0};
    sprintf(http_Request,
            "GET /v3/weather/weatherInfo?city=%s&key=463a61f47e4e4e4912294ac5c2eec98a&extensions=all&output=JSON "
            "HTTP/1.1\r\n"
            "Host: restapi.amap.com\r\n\r\n",
            city);
   
    // 发送请求
    write(tcp_socket, http_Request, strlen(http_Request));
     printf("发送请求成功\n");
    // 接收响应数据
    char buffer[4096];
    char * response = malloc(1);
    response[0]     = '\0';
    int total_size  = 0;

    while(1) {
        int size = read(tcp_socket, buffer, sizeof(buffer) - 1);
        if(size <= 0) {
            break;
        }

        buffer[size] = '\0';
        total_size += size;
        response = realloc(response, total_size + 1);
        strcat(response, buffer);
    }
    // printf("response: %s\n", response);

    close(tcp_socket);
     // 查找 JSON 数据的起始位置
    const char *json_start = strchr(response, '{');
    if (!json_start) {
        printf("未找到 JSON 数据部分\n");
        return NULL;
    }

    // 解析 JSON 数据
    cJSON *json = cJSON_Parse(json_start);
    if (!json) {
        printf("JSON 解析失败: %s\n", cJSON_GetErrorPtr());
        return NULL;
    }

    // 获取 "forecasts" 数组
    cJSON *forecasts = cJSON_GetObjectItem(json, "forecasts");
    if (!forecasts || !cJSON_IsArray(forecasts)) {
        printf("未找到 forecasts 数据\n");
        cJSON_Delete(json);
        return NULL;
    }

    // 获取第一个 forecast 对象
    cJSON *forecast = cJSON_GetArrayItem(forecasts, 0);
    cJSON *city_obj = cJSON_GetObjectItem(forecast, "city");
    cJSON *casts = cJSON_GetObjectItem(forecast, "casts");

    if (!city_obj || !casts || !cJSON_IsArray(casts)) {
        printf("未找到 city 或 casts 数据\n");
        cJSON_Delete(json);
        return NULL;
    }

    // 获取 city 名称
    const char *city_name = city_obj->valuestring;

    // 获取 casts 数组中的第一个元素
    cJSON *item = cJSON_GetArrayItem(casts, 0);
    if (!item) {
        printf("未找到第一天的天气数据\n");
        cJSON_Delete(json);
        return NULL;
    }

    cJSON *date = cJSON_GetObjectItem(item, "date");
    cJSON *week = cJSON_GetObjectItem(item, "week");
    cJSON *dayweather = cJSON_GetObjectItem(item, "dayweather");
    cJSON *daytemp = cJSON_GetObjectItem(item, "daytemp");
    cJSON *daywind = cJSON_GetObjectItem(item, "daywind");
    cJSON *daypower = cJSON_GetObjectItem(item, "daypower");

    // 分配 struct weather_list
    struct weather_list *list = malloc(sizeof(struct weather_list));
    if (!list) {
        printf("内存分配失败\n");
        cJSON_Delete(json);
        return NULL;
    }

    // 填充结构体字段
    strncpy(list->city, city_name, sizeof(list->city) - 1);
    strncpy(list->date, date->valuestring, sizeof(list->date) - 1);
    strncpy(list->week, convert_week_to_chinese(week->valuestring), sizeof(list->week) - 1);
    strncpy(list->temp, daytemp->valuestring, sizeof(list->temp) - 1);
    strncpy(list->weather, dayweather->valuestring, sizeof(list->weather) - 1);
    strncpy(list->wind, daywind->valuestring, sizeof(list->wind) - 1);
    strncpy(list->power, daypower->valuestring, sizeof(list->power) - 1);

    // 打印解析结果（可选）
    printf("城市: %s, 日期: %s, 星期: %s, 温度: %s°C, 天气: %s, 风: %s, 风力: %s\n",
           list->city, list->date, list->week, list->temp, list->weather, list->wind, list->power);

    cJSON_Delete(json);
    return list;
}
