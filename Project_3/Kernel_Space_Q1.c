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
