#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/atomic.h>

MODULE_LICENSE("GPL");

// 定義等待隊列和自旋鎖
static DECLARE_WAIT_QUEUE_HEAD(my_wait_queue);
static DEFINE_SPINLOCK(my_wait_queue_lock);

// 用來同步 clean_wait_queue 與被喚醒的執行緒
static DECLARE_WAIT_QUEUE_HEAD(done_wq);
static atomic_t done_count = ATOMIC_INIT(0);

// 將等待節點加到等待隊列尾端，以確保 FIFO
static inline void add_wait_queue_tail(wait_queue_head_t *head, wait_queue_entry_t *new)
{
    list_add_tail(&new->entry, &head->head);
}

static int enter_wait_queue(void)
{
    wait_queue_entry_t wait;

    init_waitqueue_entry(&wait, current);

    // 將等待項目加入隊列尾端
    spin_lock(&my_wait_queue_lock);
    add_wait_queue_tail(&my_wait_queue, &wait);
    set_current_state(TASK_INTERRUPTIBLE);
    pr_info("enter wait queue thread_id: %d\n", current->pid);
    spin_unlock(&my_wait_queue_lock);

    // 執行緒進入休眠，等待被喚醒
    schedule();

    // 被喚醒後恢復執行
    spin_lock(&my_wait_queue_lock);
    set_current_state(TASK_RUNNING);
    list_del_init(&wait.entry);
    pr_info("exit wait queue thread_id: %d\n", current->pid);
    spin_unlock(&my_wait_queue_lock);

    // 告知 clean_wait_queue 我已經完成印出並離開
    atomic_inc(&done_count);
    wake_up(&done_wq);

    return 1;
}

static int clean_wait_queue(void)
{
    pr_info("start clean queue ...\n");
    spin_lock(&my_wait_queue_lock);

    // 記錄目前已完成數
    int old_done = atomic_read(&done_count);

    // 逐一喚醒等待中的執行緒 (FIFO)
    while (!list_empty(&my_wait_queue.head)) {
        // 取出隊首的等待項目
        wait_queue_entry_t *first = list_first_entry(&my_wait_queue.head, wait_queue_entry_t, entry);

        // 將該項目由等待佇列中移除
        list_del_init(&first->entry);

        // 喚醒對應的執行緒
        wake_up_process(first->private);

        // 釋放鎖讓被喚醒的執行緒可以繼續往下執行並印出
        spin_unlock(&my_wait_queue_lock);

        // 等待直到該執行緒真正結束（即 done_count 增加）
        wait_event(done_wq, atomic_read(&done_count) > old_done);
        old_done = atomic_read(&done_count);

        // 再次取得鎖，繼續處理下一個等待項目
        spin_lock(&my_wait_queue_lock);
    }

    spin_unlock(&my_wait_queue_lock);
    return 1;
}

SYSCALL_DEFINE1(my_wait_queue, int, id)
{
    switch (id) {
        case 1:
            return enter_wait_queue();
        case 2:
            return clean_wait_queue();
        default:
            return -EINVAL;  // 無效參數
    }
}

