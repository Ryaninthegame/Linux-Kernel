# Project_2
[Problem](https://staff.csie.ncu.edu.tw/hsufh/COURSES/FALL2021/linux_project_2.html)
## Kernel Space
```c=
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>

static unsigned long vaddr2paddr(unsigned long vaddr){
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    unsigned long paddr = 0;
    unsigned long page_addr = 0;
    unsigned long page_offset = 0;
    struct page* page;
    struct task_struct *task;

    pgd = pgd_offset(current->mm, vaddr);
    printk("pgd_val = 0x%lx\n", pgd_val(*pgd));
    printk("pgd_index = %lu\n", pgd_index(vaddr));
    if (pgd_none(*pgd)){
        printk("not mapped in pgd\n");
        return -1;
    }
    
    p4d = p4d_offset(pgd, vaddr);
    printk("p4d_val = 0x%lx\n", p4d_val(*p4d));
    printk("p4d_index = %lu\n", p4d_index(vaddr));
    if (p4d_none(*p4d)){
        printk("not mapped in p4d\n");
        return -1;
    }

    pud = pud_offset(p4d, vaddr);
    printk("pud_val = 0x%lx\n", pud_val(*pud));
    printk("pud_index = %lu\n", pud_index(vaddr));
    if (pud_none(*pud)){
        printk("not mapped in pud\n");
        return -1;
    }

    pmd = pmd_offset(pud, vaddr);
    printk("pmd_val = 0x%lx\n", pmd_val(*pmd));
    printk("pmd_index = %lu\n", pmd_index(vaddr));
    if (pmd_none(*pmd)){
        printk("not mapped in pmd\n");
        return -1;
    }
    
    pte = pte_offset_kernel(pmd, vaddr);
    printk("pte_val = 0x%lx\n", pte_val(*pte));
    printk("pte_index = %lu\n", pte_index(vaddr));
    if (pte_none(*pte)){
        printk("not mapped in pte\n");
        return -1;
    }

    /* Page frame physical address mechanism | offset */
    page = pte_page(*pte);
    paddr = page_to_phys(page) + (vaddr & (~PAGE_MASK));
    printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
    printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);

    return paddr;
}

SYSCALL_DEFINE2(getpaddr, unsigned long, vaddr, unsigned long*, result){
    unsigned long addr;
    addr = vaddr2paddr(vaddr);
    copy_to_user(result, &addr, sizeof(unsigned long));
    return 1;
}
```
## User Space
### user_2_1
gcc user_2_1.c -lpthread -o user_2_1
```c=
#include<syscall.h>
#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

int globalVar = 0;
int bss;
unsigned long heap_addr;

void* thread1(void* args);
void* thread2(void* args);

int main(){
    int *heap = malloc(sizeof(int));
    *heap = 888;
    heap_addr = (unsigned long)heap;
    int n = 0;
    unsigned long result = 0;
    printf("=========main==============\n");
    printf("==========virtual address===========\n");
    printf("stack: 0x%-16lx\n", (unsigned long)&n);
    printf("heap: 0x%-16lx\n", (unsigned long)heap_addr);
    printf("bss: 0x%-16lx\n", (unsigned long)&bss);
    printf("data: 0x%-16lx\n", (unsigned long)&globalVar);
    printf("share_lib: 0x%-16lx\n", (unsigned long)printf);
    printf("code: 0x%-16lx\n", (unsigned long)main);
    
    printf("=========physical address===========\n");
    int a = syscall(333, (unsigned long)&n, &result);
    printf("stack: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)heap_addr, &result);
    printf("heap: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)&bss, &result);
    printf("bss: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)&globalVar, &result);
    printf("data: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)printf, &result);
    printf("share_lib: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)main, &result);
    printf("code: 0x%-16lx\n", result);

    pthread_t p1, p2;
    pthread_create(&p1, NULL, thread1, 0);
    pthread_create(&p2, NULL, thread2, 0);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    return 0;
}

void* thread1(void* args){
    int n = 0;
    int *heap = malloc(sizeof(int));
    *heap = 888;
    heap_addr = (unsigned long)heap;
    unsigned long result = 0;
    printf("============Thread 1================\n");
    printf("==========virtual address===========\n");
    printf("stack: 0x%-16lx\n", (unsigned long)&n);
    printf("heap: 0x%-16lx\n", (unsigned long)heap_addr);
    printf("bss: 0x%-16lx\n", (unsigned long)&bss);
    printf("data: 0x%-16lx\n", (unsigned long)&globalVar);
    printf("share_lib: 0x%-16lx\n", (unsigned long)printf);
    printf("code: 0x%-16lx\n", (unsigned long)main);
    
    printf("=========physical address===========\n");
    int a = syscall(333, (unsigned long)&n, &result);
    printf("stack: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)heap_addr, &result);
    printf("heap: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)&bss, &result);
    printf("bss: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)&globalVar, &result);
    printf("data: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)printf, &result);
    printf("share_lib: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)main, &result);
    printf("code: 0x%-16lx\n", result);

    return 0;
}

void* thread2(void* args){
    sleep(3);
    int n = 0;
    int *heap = malloc(sizeof(int));
    *heap = 888;
    heap_addr = (unsigned long)heap;
    unsigned long result = 0;
    printf("============Thread 2================\n");
    printf("==========virtual address===========\n");
    printf("stack: 0x%-16lx\n", (unsigned long)&n);
    printf("heap: 0x%-16lx\n", (unsigned long)heap_addr);
    printf("bss: 0x%-16lx\n", (unsigned long)&bss);
    printf("data: 0x%-16lx\n", (unsigned long)&globalVar);
    printf("share_lib: 0x%-16lx\n", (unsigned long)printf);
    printf("code: 0x%-16lx\n", (unsigned long)main);
    
    printf("=========physical address===========\n");
    int a = syscall(333, (unsigned long)&n, &result);
    printf("stack: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)heap_addr, &result);
    printf("heap: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)&bss, &result);
    printf("bss: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)&globalVar, &result);
    printf("data: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)printf, &result);
    printf("share_lib: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)main, &result);
    printf("code: 0x%-16lx\n", result);
	
    return 0;
}
```
```
=========main==============
==========virtual address===========
stack    : 0x7ffccdc4f290    
heap     : 0x558f3279c260    
bss      : 0x558f31ea9020    
data     : 0x558f31ea9014    
share_lib: 0x7f0538be0f70    
code     : 0x558f31ca789a    
=========physical address===========
stack    : 0x183a77290       
heap     : 0x136df6260       
bss      : 0x139e4f020       
data     : 0x139e4f014       
share_lib: 0x1ed869f70       
code     : 0x13aa7b89a       
============Thread 1================
==========virtual address===========
stack    : 0x7f0538b7aed0    
heap     : 0x7f0530000b20    
bss      : 0x558f31ea9020    
data     : 0x558f31ea9014    
share_lib: 0x7f0538be0f70    
code     : 0x558f31ca789a    
=========physical address===========
stack    : 0x140f8aed0       
heap     : 0x144164b20       
bss      : 0x139e4f020       
data     : 0x139e4f014       
share_lib: 0x1ed869f70       
code     : 0x13aa7b89a       
============Thread 2================
==========virtual address===========
stack    : 0x7f0538379ed0    
heap     : 0x7f0530000b40    
bss      : 0x558f31ea9020    
data     : 0x558f31ea9014    
share_lib: 0x7f0538be0f70    
code     : 0x558f31ca789a    
=========physical address===========
stack    : 0x136ed8ed0       
heap     : 0x144164b40       
bss      : 0x139e4f020       
data     : 0x139e4f014       
share_lib: 0x1ed869f70       
code     : 0x13aa7b89a
```

### user_2_2
```c=
#include<syscall.h>
#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

int globalVar = 0;
int bss;
unsigned long heap_addr;

int main(){
    int *heap = malloc(sizeof(int));
    *heap = 888;
    heap_addr = (unsigned long)heap;
    int n = 0;
    unsigned long result = 0;
    printf("=========main==============\n");
    printf("==========virtual address===========\n");
    printf("stack    : 0x%-16lx\n", (unsigned long)&n);
    printf("heap     : 0x%-16lx\n", (unsigned long)heap_addr);
    printf("bss      : 0x%-16lx\n", (unsigned long)&bss);
    printf("data     : 0x%-16lx\n", (unsigned long)&globalVar);
    printf("share_lib: 0x%-16lx\n", (unsigned long)printf);
    printf("code     : 0x%-16lx\n", (unsigned long)main);
    
    printf("=========physical address===========\n");
    int a = syscall(333, (unsigned long)&n, &result);
    printf("stack    : 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)heap_addr, &result);
    printf("heap     : 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)&bss, &result);
    printf("bss      : 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)&globalVar, &result);
    printf("data     : 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)printf, &result);
    printf("share_lib: 0x%-16lx\n", result);
    a = syscall(333, (unsigned long)main, &result);
    printf("code     : 0x%-16lx\n", result);

    char s;
    scanf("%c", &s);
    return 0;
}
```
```
=========main==============
==========virtual address===========
stack    : 0x7fff90eb2fd0    
heap     : 0x5605f4621260    
bss      : 0x5605f43e6020    
data     : 0x5605f43e6014    
share_lib: 0x7f5d1da69f70    
code     : 0x5605f41e57ea    
=========physical address===========
stack    : 0x15d29dfd0       
heap     : 0x1432b7260       
bss      : 0x15d566020       
data     : 0x15d566014       
share_lib: 0x1ed869f70       
code     : 0x132c077ea

=========main==============
==========virtual address===========
stack    : 0x7ffe694e7380    
heap     : 0x55f80d3b7260    
bss      : 0x55f80d281020    
data     : 0x55f80d281014    
share_lib: 0x7f22b667df70    
code     : 0x55f80d0807ea    
=========physical address===========
stack    : 0x147d21380       
heap     : 0x139b0a260       
bss      : 0x13689c020       
data     : 0x13689c014       
share_lib: 0x1ed869f70       
code     : 0x132c077ea
```
