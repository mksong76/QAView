#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <pthread.h>


class Monitor;

class Mutex
{
    public:
        Mutex();
        ~Mutex();

        void lock();
        bool testlock();
        void unlock();

    private:
        pthread_mutex_t m_mu;
    friend class Monitor;
};

class SharedMutex : public Mutex
{
    public:
        SharedMutex();
        ~SharedMutex();

        void    ref();
        void    unref();
    private:
        int     m_refCount;
};

class Monitor
{
    public:
        Monitor();
        Monitor(Monitor &mon);
        ~Monitor();

        void    enter();
        void    exit();
        void    signal();
        void    wait();

        void    lock() { enter(); }
        void    unlock() { exit(); }
    private:
        SharedMutex     *m_mu;
        int             *m_pRef;
        pthread_cond_t  m_cv;

        bool    ref();
        bool    unref();
};


#endif
