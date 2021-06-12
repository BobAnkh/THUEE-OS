#ifndef __KERNEL__
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define MYPIPE_MAJOR 230
#define DEVICE_NAME "MyPipe"
#define BUF_SIZE 128

MODULE_LICENSE("Dual MPL/GPL");
MODULE_AUTHOR("Shen Yixin");
MODULE_DESCRIPTION("Self-defined pipe device");

// 互斥信号量
struct semaphore mutex;

// 定义内核缓冲区和读写指示
char *buffer;
size_t pWrite = 0;
size_t pRead = 0;

static int mypipe_open(struct inode *inode, struct file *kfile)
{
    printk(KERN_EMERG " Open pipe.\n");
    return 0;
}

static int mypipe_release(struct inode *inode, struct file *kfile)
{
    printk(KERN_EMERG " Release pipe.\n");
    return 0;
}

// 将内核中的缓冲区buffer中的数据输出到用户空间中
static ssize_t mypipe_read(struct file *kfile, char __user *buf, size_t count, loff_t *fpos)
{
    size_t len;
    size_t read_len = count;
    ssize_t ret = 0;
    if (down_interruptible(&mutex))
    {
        return -ERESTARTSYS;
    }

    if (pRead <= pWrite)
    {
        len = pWrite - pRead;
        if (len < count)
        {
            read_len = len;
        }
        if (copy_to_user(buf, buffer + pRead, read_len))
        {
            printk(KERN_EMERG DEVICE_NAME " Fail to read\n");
            ret = -EFAULT;
            goto out;
        }
        pRead = (pRead + read_len) % BUF_SIZE;
    }
    else
    {
        len = BUF_SIZE - pRead + pWrite;
        if (len < count)
        {
            read_len = len;
        }
        if (read_len <= (BUF_SIZE - pRead))
        {
            if (copy_to_user(buf, buffer + pRead, read_len))
            {
                printk(KERN_EMERG DEVICE_NAME " Fail to read\n");
                ret = -EFAULT;
                goto out;
            }
            pRead = (pRead + read_len) % BUF_SIZE;
        }
        else
        {
            if (copy_to_user(buf, buffer + pRead, BUF_SIZE - pRead))
            {
                printk(KERN_EMERG DEVICE_NAME " Fail to read\n");
                ret = -EFAULT;
                goto out;
            }
            if (copy_to_user(buf + BUF_SIZE - pRead, buffer, read_len - (BUF_SIZE - pRead)))
            {
                printk(KERN_EMERG DEVICE_NAME " Fail to read\n");
                ret = -EFAULT;
                goto out;
            }
            pRead = (pRead + read_len) % BUF_SIZE;
        }
    }
    printk(KERN_EMERG DEVICE_NAME " Read, pRead:%zu, pWrite:%zu\n", pRead, pWrite);
    ret = read_len;
out:
    up(&mutex);
    return ret;
}

// 将用户程序写入内核的数据写入内核中的缓冲区buffer
static ssize_t mypipe_write(struct file *kfile, const char __user *buf, size_t count, loff_t *fpos)
{
    size_t len;
    size_t write_len = count;
    ssize_t ret = 0;
    if (down_interruptible(&mutex))
    {
        return -ERESTARTSYS;
    }

    if (pRead <= pWrite)
    {
        len = BUF_SIZE - pWrite + pRead;
        if (write_len >= len)
        {
            printk(KERN_EMERG DEVICE_NAME " Fail to write, not enough buffer, pRead:%zu, pWrite:%zu\n", pRead, pWrite);
            goto out;
        }
        if (write_len <= (BUF_SIZE - pWrite))
        {
            if (copy_from_user(buffer + pWrite, buf, write_len))
            {
                printk(KERN_EMERG DEVICE_NAME " Fail to write\n");
                ret = -EFAULT;
                goto out;
            }
            pWrite = (pWrite + write_len) % BUF_SIZE;
        }
        else
        {
            if (copy_from_user(buffer + pWrite, buf, BUF_SIZE - pWrite))
            {
                printk(KERN_EMERG DEVICE_NAME " Fail to write\n");
                ret = -EFAULT;
                goto out;
            }
            if (copy_from_user(buffer, buf + BUF_SIZE - pWrite, write_len - (BUF_SIZE - pWrite)))
            {
                printk(KERN_EMERG DEVICE_NAME " Fail to write\n");
                ret = -EFAULT;
                goto out;
            }
            pWrite = (pWrite + write_len) % BUF_SIZE;
        }
    }
    else
    {
        len = pRead - pWrite;
        if (write_len >= len)
        {
            printk(KERN_EMERG DEVICE_NAME " Fail to write, not enough buffer, pRead:%zu, pWrite:%zu\n", pRead, pWrite);
            goto out;
        }
        if (copy_from_user(buffer + pWrite, buf, write_len))
        {
            printk(KERN_EMERG DEVICE_NAME " Fail to write\n");
            ret = -EFAULT;
            goto out;
        }
        pWrite = (pWrite + write_len) % BUF_SIZE;
    }
    printk(KERN_EMERG DEVICE_NAME " Write, pRead:%zu, pWrite:%zu\n", pRead, pWrite);
    ret = write_len;

out:
    up(&mutex);
    return ret;
}

// 设备文件操作方法表
static struct file_operations mypipe_flops = {
    owner : THIS_MODULE,
    open : mypipe_open,
    release : mypipe_release,
    write : mypipe_write,
    read : mypipe_read
};

// 初始化函数
static int __init mypipe_init(void)
{
    // 注册字符设备
    int ret = register_chrdev(MYPIPE_MAJOR, DEVICE_NAME, &mypipe_flops);
    if (ret < 0)
    {
        printk(KERN_EMERG DEVICE_NAME " can't register major number. Please define another MYPIPE_MAJOR number.\n");
        goto fail;
    }
    printk(KERN_EMERG DEVICE_NAME " initialized successfully.\n");
    // 分配内核缓冲区
    buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!buffer)
    {
        ret = -ENOMEM;
        goto unregister;
    }
    memset(buffer, 0, BUF_SIZE);
    // 初始化互斥信号量为1
    sema_init(&mutex, 1);

    return 0;
unregister:
    unregister_chrdev(MYPIPE_MAJOR, DEVICE_NAME);
fail:
    return ret;
}

static void __exit mypipe_exit(void)
{
    // 注销字符设备
    unregister_chrdev(MYPIPE_MAJOR, DEVICE_NAME);
    down(&mutex);
    // 释放内存
    if (buffer)
        kfree(buffer);
    printk(KERN_EMERG DEVICE_NAME " removed successfully.\n");
    up(&mutex);
}

// 初始化模块加载
module_init(mypipe_init);
module_exit(mypipe_exit);
