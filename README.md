![image](https://github.com/user-attachments/assets/7d3c8d78-1f50-4712-b1ff-67da56d2cc6d)# 基于lvgl9.1的车载UI系统
## 运行平台
Ubuntu22.04或GEC6818开发板

## 实现功能
- 音乐播放器
- 视频播放器
- 地图显示
- 时间显示
- 天气显示
- 车俩数据可控
- 通讯录
## 运行截图
主界面

![image](https://github.com/user-attachments/assets/eda2ae3d-7bb3-4566-8a02-407eaaef9067)

音乐播放器

![image](https://github.com/user-attachments/assets/11ec2814-99d7-43e2-9aa6-8f97817097f0)

视频播放器

![image](https://github.com/user-attachments/assets/f48fc5f9-0832-4ec7-b7bb-96c214fb62ba)

地图显示

![image](https://github.com/user-attachments/assets/72303e8f-0130-4448-b84d-bc4113c478e3)

通讯录

![image](https://github.com/user-attachments/assets/7e02d0d6-0f2b-4884-95dc-be4f7bc1540a)


## 编译命令
```
make clean

make -j16
```
## 编译后运行时的文件结构
- build/bin/   存放make后 生成的可执行 main
- my_demo/data 存放的资源文件

## 运行前置步骤
- 将my_demo/data 和build/bin/main 复制到虚拟机，并保证处在同一目录下

## 运行命令
在可执行文件目录下
```
./main
```
