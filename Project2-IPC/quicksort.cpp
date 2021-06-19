#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <queue>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <algorithm>

using namespace std;

#define MAX_THREAD 20
#define DIRECT_SORT 1000
#define SORT_NUM 1000000

struct Qparam
{
    int left;
    int right;
    Qparam(int l, int r)
    {
        left = l;
        right = r;
    }
};

// 定义互斥访问锁
pthread_mutex_t mutex;

// 定义线程标识符和使用数量统计
pthread_t pID[MAX_THREAD];
int created_thread = 0;

// 定义排序任务队列和信号量
queue<Qparam> job_queue;
sem_t jobs;

// 标记排序任务全部完成
int sortedNum = 0;
bool sorted = false;
sem_t finished;

// 用于数据量小于DIRECT_SORT时, 直接调用qsort进行排序的比较函数
int cmp(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}

// 快速排序的分割函数
int partition(int *buf, Qparam param)
{
    int i = param.left;
    int j = param.right;
    int x = buf[i];
    while (i < j)
    {
        while (i < j && buf[j] >= x) // 从右向左找第一个小于x的数
            j--;
        buf[i] = buf[j];
        while (i < j && buf[i] < x) // 从左向右找第一个大于等于x的数
            i++;
        buf[j] = buf[i];
    }
    buf[i] = x;
    return i;
}

void *quicksort(void *args)
{
    int *buffer = (int *)args;
    while (true)
    {
        // 检查是否完成排序
        if (sortedNum == SORT_NUM && sorted == false)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
            pthread_mutex_lock(&mutex);
            if (sorted == false)
            {
                sorted = true;
                sem_post(&finished);
            }
            pthread_mutex_unlock(&mutex);
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        }
        // 抓取新任务
        sem_wait(&jobs);
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pthread_mutex_lock(&mutex);
        Qparam param = job_queue.front();
        cout << "[FETCH JOB] left:" << param.left << " right:" << param.right << endl;
        job_queue.pop();
        pthread_mutex_unlock(&mutex);
        if (param.left <= param.right)
        {
            // 直接进行排序
            if (param.right - param.left < DIRECT_SORT)
            {
                qsort(buffer + param.left, param.right - param.left + 1, sizeof(int), cmp);
                pthread_mutex_lock(&mutex);
                sortedNum += param.right - param.left + 1;
                cout << "[DIRECT SORT] left:" << param.left << " right:" << param.right << " sorted:" << sortedNum << endl;
                pthread_mutex_unlock(&mutex);
            }
            else
            {
                int pos = partition(buffer, param);
                pthread_mutex_lock(&mutex);
                sortedNum++;
                // 向队列中推入左作业
                job_queue.push(Qparam(param.left, pos - 1));
                sem_post(&jobs);
                cout << "[PUSH JOB-LEFT] left:" << param.left << " rightL" << pos - 1 << " sorted:" << sortedNum << endl;
                if (created_thread < MAX_THREAD)
                {
                    int res = pthread_create(&pID[created_thread], NULL, quicksort, (void *)(buffer));
                    if (res)
                    {
                        cout << "Fail to create thread " << created_thread << " [ERROR]: " << strerror(res) << endl;
                        exit(-1);
                    }
                    created_thread++;
                }

                // 向队列中推入右作业
                job_queue.push(Qparam(pos + 1, param.right));
                sem_post(&jobs);
                cout << "[PUSH JOB-RIGHT] left:" << pos + 1 << " right:" << param.right << " sorted:" << sortedNum << endl;
                if (created_thread < MAX_THREAD)
                {
                    int res = pthread_create(&pID[created_thread], NULL, quicksort, (void *)(buffer));
                    if (res)
                    {
                        cout << "Fail to create thread " << created_thread << " [ERROR]: " << strerror(res) << endl;
                        exit(-1);
                    }
                    created_thread++;
                }
                pthread_mutex_unlock(&mutex);
            }
        }

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
    }
}

void checkall_pth_finish()
{
    // 关闭所有线程
    sem_wait(&finished);
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < created_thread; i++)
    {
        pthread_cancel(pID[i]);
        pthread_join(pID[i], NULL);
    }
    cout << "All thread finish!" << endl;
    sem_post(&finished);
    pthread_mutex_unlock(&mutex);
}

int main()
{
    // 初始化互斥锁和信号量
    pthread_mutex_init(&mutex, NULL);
    sem_init(&jobs, 0, 0);
    sem_init(&finished, 0, 0);

    // 共享内存
    int *buffer;
    int fd = open("input.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        perror("[ERROR]Fail to open\n");
        exit(2);
    }
    buffer = (int *)mmap(NULL, sizeof(int) * SORT_NUM, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED)
    {
        perror("[ERROR]Fail to mmap\n");
        exit(3);
    }
    close(fd);
    // 开启第一个线程
    pthread_mutex_lock(&mutex);
    job_queue.push(Qparam(0, SORT_NUM - 1));
    sem_post(&jobs);
    cout << "[MAIN JOB] " << 0 << " " << SORT_NUM - 1 << endl;
    if (created_thread < MAX_THREAD)
    {
        int res = pthread_create(&pID[created_thread], NULL, quicksort, (void *)(buffer));
        if (res)
        {
            cout << "Fail to create thread " << created_thread << " [ERROR]: " << strerror(res) << endl;
            exit(-1);
        }
        created_thread++;
    }
    pthread_mutex_unlock(&mutex);
    checkall_pth_finish();
    cout << "Write sorted data into output.txt." << endl;
    // 将buffer中排好的数写入输出文件中
    ofstream output("output.txt", ios::out);
    for (int n = 0; n < SORT_NUM; n++)
    {
        output << buffer[n] << " ";
    }
    // 销毁互斥锁和信号量
    pthread_mutex_destroy(&mutex);
    sem_destroy(&jobs);
    sem_destroy(&finished);
    // 关闭共享内存
    if (munmap(buffer, sizeof(int) * SORT_NUM))
    {
        perror("[ERROR]Fail to munmap\n");
        exit(4);
    }
    cout << "Program finished!\nYou can run `make check` to see if the sort is correct.\n"
         << endl;
    return 0;
}