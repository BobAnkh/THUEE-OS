# THUEE-OS

THUEE Operating System course homework(including projects)

清华大学电子工程系操作系统课程作业(包括课程大作业)

## Project1-ThreadSynchronization 实验一: 进程间同步/互斥问题——银行柜员服务问题

Use `g++ BankCounter.cpp -o bankcounter.exe -std=c++11 -lpthread` or `make` to compile the program in Linux.

使用命令`g++ BankCounter.cpp -o bankcounter.exe -std=c++11 -lpthread` 或者 `make`在Linux上编译程序。

Put your test example in `input.txt`. Run `./bankcounter.exe` to perform the algorithm. Result will be put in `output.txt`.

在`input.txt`文件中放置你的测试样例。执行`./bankcounter.exe`来运行该算法。结果将会被放在`output.txt`文件中

## Project2-IPC 实验二: 高级进程间通信问题——快速排序问题

Use `make` or `make build` to build the quicksort program. Use `make generate` to generate random input data in `input.txt`(for human readable) and `input.dat`(binary, for program to use) before run quicksort main program.

使用`make`或`make build`来构建快排程序。在运行快排主程序之前，使用`make generate`来生成随机的输入数据，在`input.txt`中(以人类可读的形式存储)和`input.dat`中(以二进制形式供程序使用)。

Run `./quicksort.exe` to perform multi-thread quicksort. Results will be put in `output.txt`.

运行`./quicksort.exe`来执行多线程快排。结果会存储在文件`output.txt`中。

Use `make check` to check if the data in `output.txt` is correctly sorted.

使用`make check`来检查文件`output.txt`中的数据是否被正确排序

## Project5-Driver 实验五: 驱动程序问题——管道驱动程序开发

Use kbuild standard to write a Makefile. Use `make` or `make modules` to build the module. Use `make install` to add this module to kernel and mount the corresponding device. Use `make test` to compile the read and write test programs.

使用kbuild标准来编写Makefile。使用`make`或`make modules`命令来构建模块。使用`make install`来添加模块到内核并且挂载对应的设备。使用`make test`来编译读和写的测试程序。

Run `sudo ./writetest.exe` and enter whatever you want in the terminal, which will be writen into the pipe. Run `sudo ./readtest.exe` and enter how much bytes you want to read from the pipe.

运行`sudo ./writetest.exe`并在终端输入任何你想要输入的内容，这些内容将会被写入到管道中。运行`sudo ./readtest.exe`并输入你想要从管道中读取的字节数。

Use `make uninstall` to remove the device and kernel module.

使用`make uninstall`命令来移除设备和内核模块。
