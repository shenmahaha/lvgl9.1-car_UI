#include "../../inc/thread_pool.h"


/**
  * @brief  持有锁线程被取消，触发该函数进行处理
  * @note   将该线程的互斥锁解锁
  * @param  arg：指向线程池结构体的里面互斥锁的指针(没有具体的类型，需要后面对其进行强制转换)
  * @retval None
  */ 
void handler(void *arg) 
{  
    printf("[%lu] is ended\n", pthread_self());      // 打印正在处理线程的ID
    pthread_mutex_unlock((pthread_mutex_t *)arg);    // 解互斥锁
}  

/**
  * @brief  执行函数指针(线程函数类型)的线程函数
  * @note   这个函数只是实现了调用函数指针，并没有将具体的线程函数赋值给到这个指针，所以现在不会运行其具体的功能的
  * @param  arg：指向线程池结构体的指针(没有具体的类型，需要后面对其进行线程池结构体类型)struct thread_pool的强制转换
  * @retval None
  */ 
void* routine(void *arg)
{
    #ifdef DEBUG
        printf("[%lu] is started\n", pthread_self());      // 打印正在处理线程的ID
    #endif
    // 1、获取线程池的信息
    thread_pool_p pool = (thread_pool_p)arg;

    // 2、任务结构体指针
    struct task *task_p = NULL;

    while (1)
    {
        // 1、写遗书(将清理函数handler()压入堆栈，确保调用线程即使在持有互斥锁期间被取消，也会正确释放互斥锁)
        //====================================================//
        pthread_cleanup_push(handler, (void *)&pool->lock);
        pthread_mutex_lock(&pool->lock);        
        //====================================================//

        // 2、判断是否有任务，若无则进入条件量等待队列睡眠
        while(pool->waiting_tasks == 0 && !pool->shutdown)
            pthread_cond_wait(&pool->cond, &pool->lock);
        
        // 3、若线程池要关闭并销毁，那么线程就解锁并退出
        if(pool->waiting_tasks == 0 && pool->shutdown == true)
        {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }

        // 4、获取要运行的任务(删除任务节点)
        task_p = pool->task_list->next;             // 删除节点
        pool->task_list->next = task_p->next;       
        pool->waiting_tasks--;                      // 正在等待的任务数量减一


        // 5、销毁遗书(出栈并解锁)
        //====================================================//
        pthread_mutex_unlock(&pool->lock);  
        pthread_cleanup_pop(0);
        //====================================================//

        // 6、执行线程函数
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);   // 取消线程使能，设置此线程函数在运行期间不被取消
        (task_p->do_task)(task_p->arg);                         // 执行线程函数
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);    // 取消线程使能

        
        // 7、将当前任务节点free掉
        free(task_p);
    }

    // 8、退户此线程
    pthread_exit(NULL);
    

}

/**
  * @brief  初始化线程池
  * @note   None
  * @param  pool：          指向线程池结构体的指针
  *         threads_number： 线程池里面要初始化多少个线程
  * @retval 成功：TRUE
  *         失败：FALSE
  */ 
bool THREAD_POOL_Init(thread_pool_p pool, unsigned int threads_number)
{
    // 1、初始化了一把锁，以及条件量
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond, NULL);

    // 2、初始化线程池的基础信息
    pool->shutdown  = false;                                            // 线程池处在开机状态
    pool->task_list = malloc(sizeof(struct task));                      // 给任务节点结构体分配内存堆空间
    pool->tids      = malloc(sizeof(pthread_t)*MAX_ACTIVE_THREADS);     // 申请有最多MAX_ACTIVE_THREADS个的线程标识符的堆空间
    if (pool->task_list == NULL || pool->tids == NULL)
    {
        perror("allocate memory error!\n");
        return false;
    }
    
    pool->task_list->next   = NULL;                                     // 让线程池中的任务列表指针，指向NULL(不要让其乱指)
    pool->max_waiting_tasks = MAX_WAITING_TASKS;                        // 此线程池最大可运行的任务
    pool->waiting_tasks     =  0;                                       // 正在的等待的任务数量
    pool->active_threads    = threads_number;                           // 正在运行的线程数

    int i = 0;
    for (i = 0; i < pool->active_threads; i++)
    {
        if(pthread_create(&(pool->tids[i]), NULL, routine, (void*)pool) != 0)
        {
            perror("create threads error!\n");
            return false;
        }
        #ifdef DEBUG
            printf("[%lu]:[%s] ==> tids[%d]: [%lu] is created\n" , pthread_self(), __FUNCTION__, i, pool->tids[i]);
        #endif
    }
    
    // 3、成功返回TRUE，线程池初始化成功
    return true;
}


/**
  * @brief  将具体的线程函数，赋值到任务队列里的函数指针中
  * @note   None
  * @param  pool：    指向线程池结构体的指针
  *         do_task:  指向线程函数的指针
  *         arg：     指向线程函数的参数
  * @retval 成功：TRUE
  *         失败：FALSE
  */ 
bool THREAD_POOL_AddTask(thread_pool_p pool, void* (*do_task)(void* arg), void* arg)
{
    // 1、给任务节点结构体申请空间，并进行赋值
    struct task *new_task_p = malloc(sizeof(struct task));
    if(new_task_p == NULL)
    {
        perror("allocte memory error!\n");
        return false;
    }

    new_task_p->do_task = do_task;      // 要传入的线程函数的名字(void型指针函数指针类型)
    new_task_p->arg     = arg;          // 要传入的线程函数的参数
    new_task_p->next    = NULL;         // 任务节点的下一个指向NULL，防止其乱指

    // 2、获取互斥锁，防止以下操作被抢占
    //====================================================//
    pthread_mutex_lock(&pool->lock);
    //====================================================//

    // 3、判断当前正在等待的任务是否大于等于最大等待任务数
    if(pool->waiting_tasks >= MAX_WAITING_TASKS)
    {
        pthread_mutex_unlock(&pool->lock);          // 释放锁
        fprintf(stderr, "too many tasks\n");        // 提醒任务过多
        free(new_task_p);                           // 不再加入新的任务，将其资源删除
        return false;                               // 返回false，表示加入新任务到线程池失败了
    }

    // 4、尾插法
    struct task *task_tmp_p = pool->task_list;      // 指向任务头结点
    while(task_tmp_p->next != NULL)                 // 遍历任务链表，让task_tmp_p指向下一个，直到周到链表中的最后一个节点为止
        task_tmp_p = task_tmp_p->next;

    task_tmp_p->next = new_task_p;                  // 将新的节点，插入到链表中的末尾
    pool->waiting_tasks++;                          // 线程池等待任务数加1

    // 5、释放互斥锁
    //====================================================//
    pthread_mutex_unlock(&pool->lock);
    //====================================================//

    #ifdef DEBUG
        printf("[%lu][%s] ==》 a new task has been added.\n", pthread_self(), __FUNCTION__);
    #endif


    // 6、唤醒条件量
    pthread_cond_signal(&pool->cond);               // 唤醒线程池里的那些正在睡眠的线程

    // 7、成功返回true，表明加入任务列表成功
    return true;

}   



/**
  * @brief  增加线程池的线程的数量
  * @note   None
  * @param  pool： 指向线程池结构体的指针
            additional_threads： 你要增加的线程数	
  * @retval 成功：返回增加的线程数
            失败：-1
  */
int THREAD_POOL_AddThread(thread_pool *pool, unsigned additional_threads)
{
	// 1、如果要加入线程数为0，那就没必要添加了
	if(additional_threads == 0)
		return 0;

	// 2、现如今的线程池里的线程数量    
	unsigned total_threads = pool->active_threads + additional_threads; 
	
	// 3、将新增加的线程，也一并运行起来
	int i, actual_increment = 0;
	for(i = pool->active_threads;i < total_threads && i < MAX_ACTIVE_THREADS; i++)
	{
		if(pthread_create(&((pool->tids)[i]),NULL, routine, (void *)pool) != 0)
		{
			perror("add threads error\n");
			
			// no threads has been created, return fail
			if(actual_increment == 0)
				return -1;

			break;
		}
		
		// 已经增加的线程数
		actual_increment++; 

		#ifdef DEBUG
		printf("[%u]:[%s] ==> tids[%d]: [%u] is created.\n",(unsigned)pthread_self(), __FUNCTION__,i, (unsigned)pool->tids[i]);
		#endif
	}

	// 4、将增加线程数，添加到线程池结构体信息中
	pool->active_threads += actual_increment;
	
	// 5、返回增加的线程数
	return actual_increment;
}


/**
  * @brief  删除线程池的线程的数量(删和查)
  * @note   从线程列表的后面开始删除 
  * @param  pool： 指向线程池结构体的指针
            removing_threads： 你要删除的线程数	
  * @retval 成功：返回减少的线程数
            失败：-1
			其它：removing_threads填写0，则返回线程池的数量
  */
int THREAD_POOL_RemoveThread(thread_pool *pool, unsigned int removing_threads)
{
	// 1、如果输入的线程数为0，就返回线程池现在有几个线程
	if(removing_threads == 0)
		return pool->active_threads;
	
	// 2、现有线程池的线程数，减去输入的线程数
	int remaining_threads = pool->active_threads - removing_threads; 
    //                            5           -    6

	// 3、如果输入的线程数，大于现有的线程池的线程，就将其置为1(线程池里最少要有一条线程)
	remaining_threads = remaining_threads > 0 ? remaining_threads : 1;

	// 4、将线程进行销毁
	int i;                  
	for(i=pool->active_threads-1; i>remaining_threads-1; i--)
	{
		errno = pthread_cancel(pool->tids[i]);
		if(errno != 0)
			break;

		#ifdef DEBUG
		printf("[%u]:[%s] ==> cancelling tids[%d]: [%u]...\n",(unsigned)pthread_self(), __FUNCTION__,i, (unsigned)pool->tids[i]);
		#endif
	}
	
	// 5、判断是否真的减少了线程，是的话，返回减少的线程数 
	if(i == pool->active_threads-1)
		return -1;
	else
	{  
		pool->active_threads = i+1;
		return i+1;
	}
}

/**
  * @brief  销毁线程池
  * @note   None
  * @param  pool：指向线程池结构体的指针
  * @retval 成功：true
            失败：false
  */

bool THREAD_POOL_Destroy(thread_pool *pool)
{
	// 1、将线程池关机
	pool->shutdown = true;
	
	// 2、激活所有线程  // 是为了将线程资源回收(不激活的话就一直处在睡眠中)
	pthread_cond_broadcast(&pool->cond);


	// 3、等待线程退出
	int i;
	for(i=0; i<pool->active_threads; i++)							// 根据正在运行的线程数
	{
		errno = pthread_join(pool->tids[i], NULL);					// 将一个个线程进行回收
		if(errno != 0)
		{
			printf("join tids[%d] error: %s\n",
					i, strerror(errno));
		}
		else
			printf("[%u] is joined\n", (unsigned)pool->tids[i]);
		
	}

	// 4、将线程池的空间回收
	free(pool->task_list);
	free(pool->tids);
	free(pool);

	// 5、成功，返回true
	return true;
}
