#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define NUM_THREADS 10

#ifndef SYS_my_wait_queue
#define SYS_my_wait_queue 450
#endif

void *enter_wait_queue(void *thread_id)
{
    int tid = *(int *)thread_id;

    fprintf(stderr, "enter wait queue thread_id: %d\n", tid);

    // 呼叫系統呼叫進入等待佇列 (id = 1)
    // 此呼叫會在內核中被阻塞直到有其他執行緒呼叫 clean_wait_queue 喚醒
    if (syscall(SYS_my_wait_queue, 1) < 0) {
        perror("enter_wait_queue syscall failed");
    }

    // 系統呼叫返回後表示該執行緒已被喚醒並離開等待隊列
    fprintf(stderr, "exit wait queue thread_id: %d\n", tid);

    return NULL;
}

void *clean_wait_queue()
{
    // 呼叫系統呼叫清理等待佇列 (id = 2)
    // 將喚醒一個等待中的執行緒並等待其真正印出離開訊息後才返回
    if (syscall(SYS_my_wait_queue, 2) < 0) {
        perror("clean_wait_queue syscall failed");
    }

    return NULL;
}

int main()
{
    void *ret;
    pthread_t id[NUM_THREADS];
    int thread_args[NUM_THREADS];

    // 建立多個執行緒，並呼叫 enter_wait_queue()
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i] = i;
        if (pthread_create(&id[i], NULL, enter_wait_queue, (void *)&thread_args[i]) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    // 等待一秒讓所有執行緒都進入等待隊列
    sleep(1);
    fprintf(stderr, "start clean queue ...\n");

    // 呼叫 clean_wait_queue() 執行緒
    clean_wait_queue();

    // 等待所有執行緒結束
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(id[i], &ret);
    }

    return 0;
}
