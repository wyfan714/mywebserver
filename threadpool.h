#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>

#include "log/locker.h"
#include "mysql/sql_connection_pool.h"

template <typename T>
class threadpool
{
public:
    threadpool(int actor_model, connectionPool *connPool, int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T *request);
    bool append_with_state(T *request, bool is_write);

private:
    static void *worker(void *arg);
    void run();

private:
    int m_thread_number;        // 线程数
    int m_max_requests;         // 请求队列最大请求数
    pthread_t *m_threads;       // 线程池数组
    std::list<T *> m_workqueue; // 请求队列
    locker m_queuelocker;       // 请求队列的互斥锁
    sem m_queuestat;            // 是否有任务需要处理
    connectionPool *conn_pool;
    int actor_mode;
};

template <typename T>
threadpool<T>::threadpool(int actor_model, connectionPool *pool, int thread_number, int max_requests) : m_thread_number(thread_number), actor_mode(actor_model), conn_pool(pool), m_max_requests(max_requests), m_threads(NULL)
{
    if ((thread_number <= 0) || (max_requests <= 0))
    {
        throw std::exception();
    }
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
    {
        throw std::exception();
    }
    for (int i = 0; i < thread_number; ++i)
    {
        // printf("create the %dth thread\n", i);
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}
template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
}
template <typename T>
bool threadpool<T>::append(T *request)
{
    m_queuelocker.lock();
    if (m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}
template <typename T>
bool threadpool<T>::append_with_state(T *request, bool is_write)
{
    m_queuelocker.lock();
    if (m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    request->is_write = is_write;
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}
template <typename T>
void *threadpool<T>::worker(void *arg)
{
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}
template <typename T>
void threadpool<T>::run()
{
    while (true)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if (!request)
        {
            continue;
        }
        // reactor
        if (actor_mode == 1)
        {
            if (!request->is_write)
            {
                if (request->read())
                {
                    request->is_done = true;
                    request->mysql = conn_pool->getConnection();
                    request->process();
                    conn_pool->releaseConnection(request->mysql);
                }
                else
                {
                    request->is_done = true;
                    request->timer_flag = true;
                }
            }
            else
            {
                if (request->write())
                {
                    request->is_done = true;
                }
                else
                {
                    request->is_done = true;
                    request->timer_flag = true;
                }
            }
        }
        else
        {
            request->mysql = conn_pool->getConnection();
            request->process();
            conn_pool->releaseConnection(request->mysql);
        }
    }
}
#endif