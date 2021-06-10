#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <queue>
#include <unistd.h>

using namespace std;

#define _REENTRANT //保证pthread多线程安全

const int COUNTER_NUM = 2; // 定义柜台数量

struct Customer
{
    pthread_t pID;          //线程 id
    int id;                 //客户序号
    int arrive_time;        //客户到达的时刻
    int service_duration;   //客户需要服务的时长
    int service_start;      //服务开始的时刻
    int counter_id;         //服务柜员号
    Customer(int cus_id, int a_time, int s_duration)
    {
        pID = 0;
        id = cus_id;
        arrive_time = a_time;
        service_duration = s_duration;
    }
};

struct Counter
{
    pthread_t pID;          //线程 id
    int id;                 //柜台序号
    Counter(int count_id)
    {
        pID = 0;
        id = count_id;
    }
};

// 定义存放全部顾客和全部柜台信息的vector, 定义顾客等待队列queue
vector<Customer> customer_list;
vector<Counter> counter_list;
queue<Customer *> customer_waiting;

// 定义互斥访问锁以及顾客和柜台信号量
pthread_mutex_t mutex;
sem_t customer_sem;
sem_t counter_sem;

// 定义运行起始时间
time_t program_start_time;

// 从输入文件中读取顾客信息
void read_input(const char *filename)
{
    ifstream input(filename, ios::in);
    int id, arrive_time, service_duration;
    while (true)
    {
        input >> id >> arrive_time >> service_duration;
        if (input.fail())
            break;
        customer_list.push_back(Customer(id, arrive_time, service_duration));
    }
}

// 将程序结果写入到输出文件中
void write_output(const char *filename)
{
    ofstream output(filename, ios::out);
    for (auto customer : customer_list)
    {
        output << customer.id << ' ' << customer.arrive_time << ' ' << customer.service_start << ' ' << customer.service_start + customer.service_duration << ' ' << customer.counter_id << endl;
    }
}

// 检查是否所有顾客线程都已进行完成并关闭柜台线程
void checkall_pth_finish()
{
    // 检查所有顾客线程是否结束
    for (auto customer : customer_list)
        pthread_join(customer.pID, NULL);
    // 关闭所有柜台线程
    for (auto counter : counter_list)
    {
        pthread_cancel(counter.pID);
        pthread_join(counter.pID, NULL);
    }
    cout << "All thread finish!" << endl;
}

// 顾客线程
void *customer_thread(void *arg)
{
    Customer &customer = *(Customer *)arg;
    // 模拟顾客到来
    sleep(customer.arrive_time);

    // 顾客取号(加入等待队列)
    pthread_mutex_lock(&mutex);
    cout << "[CUSTOMER] id " << customer.id << " arrive at " << time(NULL) - program_start_time << endl;
    customer_waiting.push(&customer);
    sem_post(&customer_sem);
    pthread_mutex_unlock(&mutex);

    // 顾客等待柜员空闲后接受服务
    sem_wait(&counter_sem);
    pthread_mutex_lock(&mutex);
    cout << "[CUSTOMER] id " << customer.id << " served by " << customer.counter_id << " at " << customer.service_start << ". Now time: " << time(NULL) - program_start_time << endl;
    pthread_mutex_unlock(&mutex);
    // 模拟顾客服务过程
    sleep(customer.service_duration);
    return NULL;
}

// 柜台线程
void *counter_thread(void *arg)
{
    Counter &counter = *(Counter *)arg;
    while (true)
    {
        // 柜台等待顾客
        sem_wait(&customer_sem);
        // 在柜台线程工作期间不能取消线程
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        // 柜台叫号(从等待队列中取出顾客)
        pthread_mutex_lock(&mutex);
        Customer &customer = *customer_waiting.front();
        customer.counter_id = counter.id;
        customer.service_start = time(NULL) - program_start_time;
        cout << "[COUNTER] id " << counter.id << " serve customer " << customer.id << " at " << customer.service_start << ". Now time: " << time(NULL) - program_start_time << endl;
        customer_waiting.pop();
        sem_post(&counter_sem);
        pthread_mutex_unlock(&mutex);

        // 模拟柜台服务顾客
        sleep(customer.service_duration);
        pthread_mutex_lock(&mutex);
        cout << "[COUNTER] id " << counter.id << " finish serving customer " << customer.id << " at " << time(NULL) - program_start_time << endl;
        pthread_mutex_unlock(&mutex);

        // 解除线程取消的限制
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_testcancel();
    }
}

int main()
{
    // 初始化互斥锁和信号量
    pthread_mutex_init(&mutex, NULL);
    sem_init(&customer_sem, 0, 0);
    sem_init(&counter_sem, 0, 0);
    // 从输入文件中读取顾客信息
    cout << "Read input file..." << endl;
    read_input("input.txt");
    program_start_time = time(NULL);
    // 添加柜台
    for (int i = 0; i < COUNTER_NUM; i++)
        counter_list.push_back(Counter(i + 1));

    // 产生顾客线程和柜台线程
    for (int i = 0; i < COUNTER_NUM; i++)
    {
        int res = pthread_create(&(counter_list[i].pID), NULL, counter_thread, (void *)(&counter_list[i]));
        if (res)
        {
            pthread_mutex_lock(&mutex);
            cout << "Fail to create counter thread " << i + 1 << " [ERROR]: " << strerror(res) << endl;
            pthread_mutex_unlock(&mutex);
            exit(-1);
        }
    }
    for (int i = 0; i < customer_list.size(); i++)
    {
        int res = pthread_create(&(customer_list[i].pID), NULL, customer_thread, (void *)(&customer_list[i]));
        if (res)
        {
            pthread_mutex_lock(&mutex);
            cout << "Fail to create customer thread " << i + 1 << " [ERROR]: " << strerror(res) << endl;
            pthread_mutex_unlock(&mutex);
            exit(-1);
        }
    }

    // 检查是否所有顾客线程都已进行完成并关闭柜台线程
    checkall_pth_finish();
    // 销毁互斥锁和信号量
    pthread_mutex_destroy(&mutex);
    sem_destroy(&customer_sem);
    sem_destroy(&counter_sem);
    // 将程序结果写入到输出文件中
    cout << "Write output file..." << endl;
    write_output("output.txt");

    cout << "Program finished!" << endl;
    return 0;
}