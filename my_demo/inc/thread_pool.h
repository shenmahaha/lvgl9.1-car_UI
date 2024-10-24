/**
  ******************************************************************************
  * @file    thread_pool.h
  * @author  FZetc飞贼(方惠清)
  * @version V1.0.2
  * @date    2024.9
  * @brief   线程池
  *          
  ******************************************************************************
  * @attention
  *  注意：
  *     本文档只供学习使用，不得商用，违者必究
  *	 推广：
  *  	合作交流建议：FZetcSnitch@163.com
  *		网站建设推广：www.FZetc.com
  *  	微信公众号：  FZetc飞贼 
  *
  ******************************************************************************
  */
#ifndef __THREAD_POOL_H        // 定义以防止递归包含
#define __THREAD_POOL_H

// (1)、其它头文件
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// (2)、宏定义(函数、变量、常量)
#define MAX_WAITING_TASKS  1000
#define MAX_ACTIVE_THREADS 20

// (3)、自定义类型(结构体、联合体、枚举)	
// 1、任务节点结构体
struct task
{
    void* (*do_task)(void* arg);        // 函数指针(你要运行的线程函数)
    void* arg;                          // 线程函数参数
    struct task *next;                  // 指向链表中的下一个
};

// 2、线程池信息结构体
typedef struct thread_pool
{
    pthread_mutex_t lock;               // 互斥锁， 保护任务队列
    pthread_cond_t  cond;               // 条件变量， 同步所有线程

    bool shutdown;                      // 线程销毁标记
    struct task *task_list;             // 任务链表(队列形式)
    pthread_t *tids;                    // 线程ID存放的位置

    unsigned int max_waiting_tasks;     // 最大可运行的任务数
    unsigned int waiting_tasks;         // 现在等待的任务还有几个  
    unsigned int active_threads;        // 当前活跃的线程个数

}thread_pool, *thread_pool_p;


// (4)、函数声明
extern bool THREAD_POOL_Init(thread_pool_p pool, unsigned int threads_number);
extern bool THREAD_POOL_AddTask(thread_pool_p pool, void* (*do_task)(void* arg), void* arg);
extern int THREAD_POOL_AddThread(thread_pool *pool, unsigned additional_threads);
extern int THREAD_POOL_RemoveThread(thread_pool *pool, unsigned int removing_threads);
extern bool THREAD_POOL_Destroy(thread_pool *pool);
// (5)、全局变量(声明)

// (6)、static全局变量(定义)

// (7)、static全局函数(实现)


#endif




