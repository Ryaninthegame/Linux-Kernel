## Problem
[Link](https://staff.csie.ncu.edu.tw/hsufh/COURSES/FALL2021/linux_project_2.html)
### Question 1
1. Requirement_1 : 
	* In this project, for both Questions 1 and 2, you need to write a new system call my_get_physical_addresses(unsigned int * initial, int len_vir, unsigned int * result, int len_phy) so that a process can use it to get the physical addresses of some virtual addresses.
	* The return value of this system call is either 0 or a positive value. 0 means that an error occurs when executing this system call. A positive value means the system call is executed successfully.
	* The first argument of this system call is the address of an unsigned integer array. Each element of the array stores a virtual address of a process.
The second argument of this system call is the number of elements in the array.
	* The third argument is the address of an unsigned integer array. Each element with index i of this array stores the physical address of the virtual address stored as element i in the array pointed by the first argument.
	* The fourth argument is the number of elements stored in the array pointed by the third argument.
	
2. Requirement_2 : 
/*prototype of the new system call is as follows:*/     
int my_get_physical_addresses(unsigned int * initial, int len_vir, unsigned int * result, int len_phy)

3. Requirement_3 :
Write a multi-thread program with three threads using the new system call to show how the following memory areas are shared by these threads. Your program must use variables with storage class __thread. The memory areas include code segments, data segments, BSS segments, heap segments, libraries, stack segments, and thread local storages. You need to draw a figure as follows to show your results.
![](https://img.cntofu.com/book/note/linux_system/images/thread-stack.png)

4. Hint : 
	* Two threads show a physical memory cell (one byte) if both of them have a virtual address that is translated into the physical address of the memory cell.
	* The kernel usually does not allocate physical memories to store all code and data of a process when the process starts execution. Hence, if you want kernel to allocate physical memories to a piece of code, execute the code first. If you want kernel to allocate physical memories to a variable, access the variable first.
	* Inside the Linux kernel, you need to use function copy_from_user() and function copy_to_user() to copy data from/to a user address buffer.
	* Check the "Referenced Material" part of the Course web site to see how to add a new system call in Linux.

### Question 2
* Write a program with the new system call my_get_physical_addresses(unsigned int * initial, int len_vir, unsigned int * result, int len_phy) to check how memory areas are shared by two processes that execute this program simultaneously.
* Hint:
	* When making your check, both related processes must be in progress. Hence you may need to use function sleep() to guarantee this requirement.
	* Inside the Linux kernel, you need to use function copy_from_user() and function copy_to_user() to copy data from/to a user address buffer.
	* Check the "Referenced Material" part of the Course web site to see how to add a new system call in Linux.

## Q&A
1. 當程式調用memory allocation的方法，如：malloc、new時，OS會如何處理？
答: Process分配memory有兩種方式：分別由brk和mmap完成。brk是將data段的最高地址指針_edata往高地址推，mmap是在process的虛擬地址空間中，找一塊空閒的虛擬空間
2. 說明multithread和multiprocess記憶體共用情形並畫圖。
答: multithread除了stack與heap段不共用外，其餘均共用。 multiprocess除了code與share library段共用外，其餘均不共用。
![](https://i.imgur.com/DX8u3Up.jpg)


3. 說明當程式執行pthread_create時，system call的執行流程。
答: 
* 使用strace看使用到的system call(mmap, mprotect, clone)
* 接著看clone的code，呼叫到_do_fork
```
SYSCALL_DEFINE5(clone, unsigned long, clone_flags, unsigned long, newsp,
                int __user *, parent_tidptr,
                int __user *, child_tidptr,                                                                                          
                unsigned long, tls)         
{                
       return _do_fork(clone_flags, newsp, 0, parent_tidptr, child_tidptr, tls);
}      
```
* 接著看_do_fork，呼叫到copy_process來複製一個新的process
```
/*
 *  Ok, this is the main fork-routine.
 *
 * It copies the process, and if successful kick-starts
 * it and waits for it to finish using the VM if required.
 */
long do_fork(unsigned long clone_flags,
	      unsigned long stack_start,
	      struct pt_regs *regs,
	      unsigned long stack_size,
	      int __user *parent_tidptr,
	      int __user *child_tidptr)
{
	struct task_struct *p; // 宣告一個 process descriptor
	int trace = 0;
	struct pid *pid = alloc_pid(); // 要求一個 PID 給新的 process 使用
	long nr;

	if (!pid)
		return -EAGAIN;
	nr = pid->nr;
	if (unlikely(current->ptrace)) {
		trace = fork_traceflag (clone_flags);
		if (trace)
			clone_flags |= CLONE_PTRACE;
	}
	// 呼叫 copy_process()，以複制出新的 process
	p = copy_process(clone_flags, stack_start, regs, stack_size, parent_tidptr, child_tidptr, nr);
	/*
	 * Do this prior waking up the new thread - the thread pointer
	 * might get invalid after that point, if the thread exits quickly.
	 */
	if (!IS_ERR(p)) {
		struct completion vfork;

		if (clone_flags & CLONE_VFORK) {
			p->vfork_done = &vfork;
			init_completion(&vfork);
		}

		if ((p->ptrace & PT_PTRACED) || (clone_flags & CLONE_STOPPED)) {
			/*
			 * We'll start up with an immediate SIGSTOP.
			 */
			sigaddset(&p->pending.signal, SIGSTOP);
			set_tsk_thread_flag(p, TIF_SIGPENDING);
		}

		if (!(clone_flags & CLONE_STOPPED))
			wake_up_new_task(p, clone_flags);
		else
			p->state = TASK_STOPPED;

		if (unlikely (trace)) {
			current->ptrace_message = nr;
			ptrace_notify ((trace << 8) | SIGTRAP);
		}

		if (clone_flags & CLONE_VFORK) {
			wait_for_completion(&vfork);
			if (unlikely (current->ptrace & PT_TRACE_VFORK_DONE))
				ptrace_notify ((PTRACE_EVENT_VFORK_DONE << 8) | SIGTRAP);
		}
	} else {
		free_pid(pid);
		nr = PTR_ERR(p);
	}
	return nr;
}
```
* copy_process開始複製current給新的process：
```
	if ((retval = copy_semundo(clone_flags, p)))
		goto bad_fork_cleanup_audit;
	if ((retval = copy_files(clone_flags, p)))
		goto bad_fork_cleanup_semundo;
	if ((retval = copy_fs(clone_flags, p)))
		goto bad_fork_cleanup_files;
	if ((retval = copy_sighand(clone_flags, p)))
		goto bad_fork_cleanup_fs;
	if ((retval = copy_signal(clone_flags, p)))
		goto bad_fork_cleanup_sighand;
	if ((retval = copy_mm(clone_flags, p)))
		goto bad_fork_cleanup_signal;
	if ((retval = copy_keys(clone_flags, p)))
		goto bad_fork_cleanup_mm;
	if ((retval = copy_namespace(clone_flags, p)))
		goto bad_fork_cleanup_keys;
	retval = copy_thread(0, clone_flags, stack_start, stack_size, p, regs);
	if (retval)
		goto bad_fork_cleanup_namespace;
```
    流程依序為：clone->_do_fork->copy_process->copy_mm
4. virtual address 什麼時候連上 physical address ?
答：在第一次訪問到資料時，對應的PTE的valid bit是沒有被設置的，此時為page fault。當遇到page fault時，MMU會將控制權還給OS，這時交由Page fault handler來處理，handler會選擇某個在DRAM中的page來替換。(page fault handler)

5. current macro 在不同thread用到時是同個task struct?
答: 不同thread皆有自己的pid，因此不同thread的task struct不會一樣。



## Reference

1. [Stack Overflow](https://stackoverflow.com/questions/41090469/linux-kernel-how-to-get-physical-address-memory-management?fbclid=IwAR1hAHO4eZy7BhUIFCfVxtTsBkP5njKV31jj7kRU1p10Y3mqefzmfLFiOic)
2. [Module教學](https://jerrynest.io/how-to-write-a-linux-kernel-module/)
3. [Page Table Management](https://www.kernel.org/doc/gorman/html/understand/understand006.html?fbclid=IwAR3gGfrQmiSeGSsTO2b_0cHaneq1TVs-lnjimZLG2MaTojJM-25CCHQG8ZU)
4. [copy_process()](https://www.jollen.org/blog/2007/01/process_creation_5_copy_process.html?fbclid=IwAR0Hrlti9YLluxAwbRxj9DMzpytCN_GSAPQB2YHMKH1EtGLWrsEFjH6vGeA)
5. [內存的分配與管理](https://codertw.com/%E7%A8%8B%E5%BC%8F%E8%AA%9E%E8%A8%80/676150/)
