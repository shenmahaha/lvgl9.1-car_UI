#include "../../inc/movie.h"

static int fifo_fd;
static char movie_cmd_buf[256] = {0};
static char mp4_buf[256] = {0};
static FILE *mplayer_output; // 定义 mplayer 输出流
int movie_flag = 1;
extern int32_t music_volume; // 音量

// 0、打开管道文件，让命令得以从此管道输入
int open_mplayer_fifo(void)
{
	// 0.判断文件是否存在
	// 存在   -->  0  --> 假  -->if的内容不执行
	// 不存在 --> -1  --> 真  -->if的内容执行
	if (access("/tmp/fifo", F_OK))
	{
		// 1.创建管道文件c
		int ret = mkfifo("/tmp/fifo", 0777);
		if (ret == -1)
		{
			printf("mkfifo error!\n");
			return 0;
		}
	}

	// 2.访问管道文件
	fifo_fd = open("/tmp/fifo", O_RDWR);
	if (fifo_fd < 0)
	{
		printf("open fifo error!\n");
		return 0;
	}

	return 0;
}

// 1、关闭管道
int close_mplayer_fifo(void)
{
	close(fifo_fd);
	return 0;
}

// 2、播放视频
int start_movie(int movie_x, int movie_y, int movie_wide, int movie_high, char *movie_path)
{
	bzero(movie_cmd_buf, sizeof(movie_cmd_buf)); // 清0
	sprintf(movie_cmd_buf, "mplayer -slave -quiet -input file=/tmp/fifo -geometry %d:%d -zoom -x %d -y %d %s &",
			movie_x, movie_y, movie_wide, movie_high, movie_path);
	// system(movie_cmd_buf);
	mplayer_output = popen(movie_cmd_buf, "r"); // 使用 popen 打开 mplayer，并获取输出流
	if (mplayer_output == NULL)
	{
		printf("启动 mplayer 失败！\n");
		exit(1);
	}
	char cmd[256] = {0};
	sprintf(cmd, "echo volume %d 1 > /tmp/fifo",music_volume); // 降低初始音量
	 system(cmd); // 降低初始音量
	return 0;
}

// 3、视频音量
int adjust_movie_volume(int movie_volume_num)
{
	bzero(movie_cmd_buf, sizeof(movie_cmd_buf));
	sprintf(movie_cmd_buf, "volume %d\n", movie_volume_num);
	write(fifo_fd, movie_cmd_buf, strlen(movie_cmd_buf));
	return 0;
}

// 4、视频快进、快退
int movie_forward_behind(int movie_fw_bh_num)
{
	bzero(movie_cmd_buf, sizeof(movie_cmd_buf));
	sprintf(movie_cmd_buf, "seek %d\n", movie_fw_bh_num);
	write(fifo_fd, movie_cmd_buf, strlen(movie_cmd_buf));
	return 0;
}

// 5、视频暂停/继续
int stop_movie()
{

	bzero(movie_cmd_buf, sizeof(movie_cmd_buf));
	strcpy(movie_cmd_buf, "pause\n");
	write(fifo_fd, movie_cmd_buf, strlen(movie_cmd_buf));
	fsync(fifo_fd);
	return 0;
}

// 6、视频退出
int quit_movie()
{
	bzero(movie_cmd_buf, sizeof(movie_cmd_buf));
	strcpy(movie_cmd_buf, "quit 0\n");
	write(fifo_fd, movie_cmd_buf, strlen(movie_cmd_buf));
	return 0;
}

// 7、视频静音设置1为静音开启，0为关闭
int mute_movie(int mute_num)
{
	bzero(movie_cmd_buf, sizeof(movie_cmd_buf));
	sprintf(movie_cmd_buf, "mute %d\n", mute_num);
	write(fifo_fd, movie_cmd_buf, strlen(movie_cmd_buf));
	return 0;
}

// 获取当前视频播放时间（进度条）
float get_movie_position()
{
	char command[256] = {0};
	char response[256] = {0};

	// 发送命令请求当前播放时间
	sprintf(command, "get_time_pos\n");
	write(fifo_fd, command, strlen(command));

	// 循环读取 mplayer 的输出
	while (fgets(response, sizeof(response), mplayer_output))
	{
		// 如果读取到 "ANS_TIME_POSITION" 字符串，说明是我们想要的时间信息
		if (strstr(response, "ANS_TIME_POSITION="))
		{
			float current_time = 0;
			sscanf(response, "ANS_TIME_POSITION=%f", &current_time);
			return current_time; // 返回当前时间
		}
	}

	return -1; // 如果未能获取时间，则返回 -1
}

// 获取视频总时长
float get_movie_duration()
{
	char command[256] = {0};
	char response[256] = {0};

	// 发送命令请求视频总时长
	sprintf(command, "get_time_length\n");
	write(fifo_fd, command, strlen(command));

	// 循环读取 mplayer 的输出
	while (fgets(response, sizeof(response), mplayer_output))
	{
		// 如果读取到 "ANS_LENGTH" 字符串，说明是我们想要的时长信息
		if (strstr(response, "ANS_LENGTH="))
		{
			float duration = 0;
			sscanf(response, "ANS_LENGTH=%f", &duration);
			return duration; // 返回视频总时长
		}
	}

	return -1; // 如果未能获取时长，则返回 -1
}
