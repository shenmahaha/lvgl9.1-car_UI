#include "../../../inc/TCP_client.h"
#include "../../../inc/home_win.h"
extern pthread_mutex_t lock;
int tcp_socket = 0;
int tcp_client_init()
{
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(8080);
    addr.sin_addr.s_addr = inet_addr("192.168.73.47");
    int ret              = connect(tcp_socket, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        perror("链接服务器失败\n");
        return -1;
    } else {
        printf("链接服务器成功\n");
    }

    while(1) {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(tcp_socket, &set);
        printf("开始监视...\n");
        int ret = select(tcp_socket + 1, &set, NULL, NULL, NULL); // 监视可读事件
        if(ret > 0) {
            printf("有活跃的描述符请处理\n");
            if(FD_ISSET(tcp_socket, &set)) {
                char buf[1024] = {0};
                int size       = read(tcp_socket, buf, sizeof(buf));
                if(size <= 0) {
                    printf("服务器断开连接！\n");
                    break;
                }
                printf("读取到数据大小：%d, 内容:%s \n", size, buf);
                // 接收格式speed:xxx
                char * p = strtok(buf, ":");
                if(strcmp(p, "speed") == 0) {
                    p = strtok(NULL, ":");
                    lv_label_set_text(speed_label, p); // 更新速度
                } else if(strcmp(p, "mileage") == 0) {
                    p = strtok(NULL, ":");
                    printf("p:%s\n", p);
                    lv_label_set_text(mileage_label_val, p); // 更新里程
                } else if(strcmp(p, "electricity") == 0) {
                    p = strtok(NULL, ":");
                    lv_label_set_text(electricity_label_val, p); // 更新电量
                }
            }
        }
    }
    close(tcp_socket);
}