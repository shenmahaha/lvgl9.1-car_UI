#ifndef __TCP_GET_WEATHER_H__ // 定义以防止递归包含
#define __TCP_GET_WEATHER_H__

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <cjson/cJSON.h>


struct weather_list
{
    char city[80];
    char date[80];
    char week[80];
    char temp[80];
    char weather[80];
    char wind[80];
    char power[80];
};

extern struct weather_list *  tcp_get_weather(char *city);

#endif