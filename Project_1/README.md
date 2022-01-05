# A Writeup of Project 1
![](https://i.imgur.com/3fs2dT0.jpg)

[task_struct](https://elixir.bootlin.com/linux/v4.14/source/include/linux/sched.h#L519) 結構如上
## Task
1. [寫一個 system call](https://blog.kaibro.tw/2016/11/07/Linux-Kernel%E7%B7%A8%E8%AD%AF-Ubuntu/) 從 kernel space 找出指定 process(task) 的 code(text) 段的位址

```warning
每個版本編譯 kernel 方法不盡相同，請找適合自己版本的方法
```

2. user 段程式利用 system call 傳入 pid 並取得結果放至 user space 的 buffer 上，再輸出至 terminal 上
