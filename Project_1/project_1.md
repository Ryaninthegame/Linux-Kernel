# Project_1
[Problem](https://hackmd.io/t2tM_VFoTkyike6zlCmLWA?both)
## Kernel Space
```c=
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>

struct data_segment{
    unsigned long start_code;
    unsigned long end_code;
};

SYSCALL_DEFINE2(pass_kernel_data, pid_t, user_pid, void* __user, user_address){
    struct data_segment my_data_segment;
    struct task_struct *task;

    for_each_process(task){
        if(task->pid == user_pid){
            my_data_segment.start_code = task->mm->start_code;
            my_data_segment.end_code = task->mm->end_code;

            printk("pid       : %d", user_pid);
            printk("start code: %lx", my_data_segment.start_code);
            printk("end   code: %lx", my_data_segment.end_code);

            copy_to_user(user_address, &my_data_segment, sizeof(struct data_segment));
        }
    }

    return 1;
}
```
## User Space
```c=
#include <syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

struct data_segment{
    unsigned long start_code;
    unsigned long end_code;
};

int main(){
    struct data_segment my_data_segment;

    int a = syscall(334, getpid(), (void*)&my_data_segment);

    printf("Start: %lx\nEnd  : %lx\n", my_data_segment.start_code, my_data_segment.end_code);

    return 0;
}
```
