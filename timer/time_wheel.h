#ifndef TIME_WHEEL_TIMER
#define TIME_WHEEL_TIMER

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include "../log/log.h"
#define BUFFER_SIZE 64

class tw_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer *timer;
};

class tw_timer
{
public:
    tw_timer(int rot, int ts) : next(NULL), prev(NULL), rotation(rot), time_slot(ts)
    {
    }

public:
    int rotation;  // 定时器在时间轮转多少圈后生效
    int time_slot; // 记录定时器对应哪个槽
    void (*cb_func)(client_data *);
    client_data *user_data;
    tw_timer *next;
    tw_timer *prev;
};

class time_wheel
{
public:
    time_wheel() : cur_slot(0)
    {
        for (int i = 0; i < N; ++i)
        {
            slots[i] = NULL;
        }
    }
    ~time_wheel()
    {
        // 遍历每个槽
        for (int i = 0; i < N; ++i)
        {
            tw_timer *tmp = slots[i];
            while (tmp)
            {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }
    tw_timer *add_timer(int timeout)
    {
        if (timeout < 0)
        {
            return NULL;
        }
        int ticks = 0;
        if (timeout < SI)
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / SI;
        }
        int rotation = ticks / N;
        int ts = (cur_slot + (ticks % N)) % N;
        tw_timer *timer = new tw_timer(rotation, ts);
        // 第ts个槽中无定时器,插入头节点
        if (!slots[ts])
        {
            LOG_INFO("add timer, rotation is %d, ts is %d, cur_slot is %d", rotation, ts, cur_slot);
            slots[ts] = timer;
        }
        else
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }
    void del_timer(tw_timer *timer)
    {
        if (!timer)
        {
            return;
        }
        int ts = timer->time_slot;
        if (timer == slots[ts])
        {
            slots[ts] = slots[ts]->next;
            if (slots[ts])
            {
                slots[ts]->prev = NULL;
            }
            delete timer;
        }
        else
        {
            timer->prev->next = timer->next;
            if (timer->next)
            {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }
    void tick()
    {
        tw_timer *tmp = slots[cur_slot];
        LOG_INFO("current slot is %d", cur_slot);
        while (tmp)
        {
            LOG_INFO("tick the timer once");
            if (tmp->rotation > 0)
            {
                tmp->rotation--;
                tmp = tmp->next;
            }
            else
            {
                tmp->cb_func(tmp->user_data);
                if (tmp == slots[cur_slot])
                {
                    LOG_INFO("delete header in cur_slot");
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if (slots[cur_slot])
                    {
                        slots[cur_slot]->prev = NULL;
                    }
                    tmp = slots[cur_slot];
                }
                else
                {
                    tmp->prev->next = tmp->next;
                    if (tmp->next)
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer *tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        cur_slot = ++cur_slot % N;
    }

private:
    // 时间轮上槽的数目
    static const int N = 60;
    // 每SI秒时间轮转动一次
    static const int SI = 10;
    tw_timer *slots[N];
    int cur_slot;
};

#endif