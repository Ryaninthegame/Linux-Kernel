# Linux-Kernel
National Central University Linux Operating System Project

## Contributor
Ryan Wang, Cheng-Kai Wang, Yung-Peng Hsu

## Linux Kernel 編譯
* Linux 版本：ubuntu 18.04
* Kernel 版本：Kernel 4.15

### 前置作業
* [安裝VMware](https://blog.xuite.net/yh96301/blog/341981056-VMware+Workstation+16+Player%E5%AE%89%E8%A3%9DUbuntu+20.04%3E)
* [安裝VMware tools](https://www.youtube.com/watch?v=fSOvnlEowLA)
    * 安裝這個才能讓 VMware 變成全螢幕，否則螢幕太小。
* [下載Linux Kernel](http://ftp.ntu.edu.tw/linux/kernel/v4.x/?fbclid=IwAR0fq2e0T60YB54O2xZGouyQ33z4o_kxkmElhdn-y9CqIZnq2bc2lwVIdwk)
    * 下載「linux-4.15.tar.gz」(Ctrl+F搜尋比較快)
    * 解壓縮
        ```
        tar zxvf linux-4.15.tar.gz
        ```

### 編譯 Kernel 步驟
若沒有修改到tbl file（沒有新增新的system call）直接執行 8 和 9 即可，會加速 compile 的時間
```
$ cd linux-4.15
```
1. 把要編寫的 system call 放在這裡 (Kernel space)
    ```
    $ mkdir mycall
    $ cd mycall
    ```
2. 將 system call 的程式編寫進去，以 project_1 當作例子
    ```
    $ vim helloworld.c
    ```
    ```c=
    // helloworld.c 內的 code
    #include <linux/kernel.h>
    #include <linux/string.h>
    #include <linux/uaccess.h>
    #include <linux/init_task.h>
    #include <linux/syscalls.h>

    struct data_segment{
        unsigned long start_code;
        unsigned long end_code;
    };

    SYSCALL_DEFINE2(sys_helloworld, pid_t, user_pid, void* __user, user_address){
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
3. 在同一層內，修改 Makefile
    ```
    $ vim Makefile
    加入下列程式
    obj-y := helloworld.o
    ```
4. 回到上層目錄，修改 Makefile
    * 這是為了告訴它，我們新的system call的source files在mycall資料夾裡
    ```
    $ cd ..
    $ vim Makefile

    找到下列這行，在最後面新增 mycall/
    core-y += kernel/ mm/ fs/ ipc/ security/ crypto/ block/
    ↓
    core-y += kernel/ mm/ fs/ ipc/ security/ crypto/ block/ mycall/
    
    若是在其他資料夾也有 system call，繼續往後串就可以惹。
    例如跟 mycall 同一層有另外一個資料夾 project_1
    core-y += kernel/ mm/ fs/ ipc/ security/ crypto/ block/ mycall/ project_1
    ```
5. 再來要把我們新增的 system call 加到 system call table 裡
    * 下列指示適用在 64 位元的 OS
    * 在 64 位元最後新增一行，原本 system call 只到 332 號，所以這邊就填 333 號
    * 第三個為檔名，第四個為 「sys_」+「function 名稱」 (參照第二步的 code)
    ```
    $ vim arch/x86/entry/syscalls/syscall_64.tbl
    333   64  helloworld  sys_helloworld
    ```
6. 接著編譯kernel前要裝一些套件
    ```
    $ sudo apt install build-essential libncurses-dev libssl-dev libelf-dev bison flex -y
    $ sudo apt-get install libncurses5-dev
    $ sudo apt install gcc-multilib
    ```
7. 還有設定檔 (先把terminal文字縮小再執行這行）
    ```
    // 這邊我都用預設的，輸入後會跳出視窗，連續按兩下 Esc 即可。
    $ sudo make menuconfig
    $ sudo make oldconfig
    ```
8. 開始 Compile
    ```
    $ sudo make -j8
    ```
9. 編完之後，安裝到作業系統上
    ```
    $ sudo make modules_install install
    ```
10. 更改 grub 設定重開時可以選擇我們的 kernel 
    * 下列選擇一種執行，我自己做時是用第一種，跟同學做時是遇到第二種。
    * 重新開機後選擇「Advanced options for Ubuntu」，之後可以看到自己編譯的版本，選「Ubuntu, with Linux 4.15.0 (recovery mode)」
    ```
    $ sudo vim /etc/default/grub
    把 GRUB_TIMEOUT_STYLE 改成 countdown，還有 GRUB_TIMEOUT 改成 10
    GRUB_TIMEOUT_STYLE=countdown
    GRUB_TIMEOUT=10
    $ sudo update-grub
    重開機後狂按 shift，即可進到選擇 kernel 的畫面
    ```
    ```
    $ sudo vim /etc/default/grub
    像我一樣找到下面這兩行，前面加#，把他們註解掉
    #GRUB_HIDDEN_TIMEOUT=0
    #GRUB_HIDDEN_TIMEOUT_QUIET=true
    然後更新
    $ sudo update-grub
    ```
11. 開機後，在 user space 寫個程式來測試看看有沒有成功，若有看到第二步內 printk 的內容，代表成功。
    ```
    $ vim test.c
    要先在 user space 執行 system call 後，「dmesg」才會有 output。 
    $ gcc -o test test.c
    $ ./test
    $ dmesg
    ```
    ```c=
    // test.c 內的 code
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
    
### Debug
* [Link](https://blog.csdn.net/qq_36393978/article/details/118157426?fbclid=IwAR1vWkuHQ9dXaUqxNOKxnZzsMGSHeEc0555KZ6Hbn0ScY2z0sVdhocgC68A)
    ```
    No rule to make target ‘debian/canonical-certs.pem‘, needed by 
    ‘certs/x509_certificate_list‘
    ```
