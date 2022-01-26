# Project_3
[Problem](https://staff.csie.ncu.edu.tw/hsufh/COURSES/FALL2021/linux_project_3.html)
## Question 1
### Kernel Space
```c=
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>
#include <asm/errno.h>

SYSCALL_DEFINE1(context_switches, unsigned int*, w){
        unsigned int switch_counts;
        switch_counts = current->nivcsw + current->nvcsw;
        if(copy_to_user(w, &switch_counts, sizeof(unsigned int))){
                return -EFAULT;
        }

        return 0;
}

```

### User Space
```c=
#include <stdio.h>
#include <syscall.h>
#include<unistd.h>
#define  NUMBER_OF_ITERATIONS     99999999
int main (){
        int i;
        int v;
        int t=2;
        int u=3;
        unsigned int w;

        // do nop
        for(i=0; i<NUMBER_OF_ITERATIONS; i++)
            v=(++t)*(u++);

        if(syscall(335, &w) != 0)
            printf("Error!\n");
        else
            printf("This process encounters %u times context switches.\n", w);
}

```
![](https://i.imgur.com/jhMzlh1.png)


## Question 2
### Modify Kernel Code
* 在 linux-4.15/include/linux/sched.h 的 1090 行處，新增下列程式碼
    ```c=
    // 在 linux-4.15/include/linux/sched.h 的 1090 行處，新增下列程式碼

    #ifdef CONFIG_LIVEPATCH
            int patch_state;
    #endif
    #ifdef CONFIG_SECURITY
            /* Used by LSM modules for access restriction: */
            void                            *security;
    #endif
            unsigned int enter_queue_counter; // here

            /*
             * New fields for task_struct should be added above here, so that
             * they are included in the randomized portion of task_struct.
             */
            randomized_struct_fields_end

            /* CPU-specific state of this task: */
            struct thread_struct            thread;

            /*
             * WARNING: on x86, 'thread_struct' contains a variable-sized
             * structure.  It *MUST* be at the end of 'task_struct'.
             *
             * Do not put anything below here!
             */
    };
    ```

* 在 linux-4.15/kernel/fork.c 的 1222 行處，新增下列程式
    ```c=
    /*
     * Mark the task runnable and perform wakeup-preemption.
     */
    static void ttwu_do_wakeup(struct rq *rq, struct task_struct *p, int wake_flags,
                               struct rq_flags *rf)
    {
            check_preempt_curr(rq, p, wake_flags);
            p->enter_queue_counter++; //here
            p->state = TASK_RUNNING;
            trace_sched_wakeup(p);

    #ifdef CONFIG_SMP
            if (p->sched_class->task_woken) {
                    /*
                     * Our task @p is fully woken up and running; so its safe to
                     * drop the rq->lock, hereafter rq is only used for statistics.
                     */
    ```
    
* 在 linux-4.15/kernel/sched/core.c 的 1672 行處，新增下列程式碼
    ```c=
    /*
     * Mark the task runnable and perform wakeup-preemption.
     */
    static void ttwu_do_wakeup(struct rq *rq, struct task_struct *p, int wake_flags,
                               struct rq_flags *rf)
    {
            check_preempt_curr(rq, p, wake_flags);
            p->enter_queue_counter++; //here
            p->state = TASK_RUNNING;
            trace_sched_wakeup(p);

    #ifdef CONFIG_SMP
            if (p->sched_class->task_woken) {
                    /*
                     * Our task @p is fully woken up and running; so its safe to
                     * drop the rq->lock, hereafter rq is only used for statistics.
                     */
    ```
### Kernel Space
```c=
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>
#include <asm/errno.h>

SYSCALL_DEFINE1(enter_queue, unsigned int*, w){
        unsigned int enter_queue_counts;
        enter_queue_counts = current->enter_queue_counter;
        if(copy_to_user(w, &enter_queue_counts, sizeof(unsigned int))){
                return -EFAULT;
        }

        return 0;
}
```

### User Space
```c=
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

#define  NUMBER_OF_IO_ITERATIONS     6
#define  NUMBER_OF_ITERATIONS        99999999

int main (){
        char c;
        int i,t=2,u=3,v;
        unsigned int w;

        for(i=0; i<NUMBER_OF_IO_ITERATIONS; i++){
                v=1;
                c = getchar();
        }

        for(i=0; i<NUMBER_OF_ITERATIONS; i++)
                v=(++t)*(u++);

        if(syscall(335, &w)!=0)
                printf("Error (1)!\n");
        else
                printf("This process encounters %u times context switches.\n", w);

        if(syscall(336, &w)!=0)
                printf("Error (2)!\n");
        else
                printf("This process enters a wait queue %u times.\n", w);


        for(i=0; i<NUMBER_OF_IO_ITERATIONS; i++){
                v=1;
                printf("I love my home.\n");
        }


        if(syscall(336, &w)!=0)
                printf("Error (3)!\n");
        else
                printf("This process enters a wait queue %u times.\n", w);
}

```
![](https://i.imgur.com/hnGnB4Q.png)

![](https://i.imgur.com/2AzeWox.png)

![](https://i.imgur.com/NqmtRxn.png)

