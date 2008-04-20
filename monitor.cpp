#include "monitor.h"
#include <pthread.h>


Mutex::Mutex()
{
    pthread_mutex_init( &m_mu, NULL );
}

Mutex::~Mutex()
{
    pthread_mutex_destroy( &m_mu );
}

void Mutex::lock()
{
    pthread_mutex_lock( &m_mu );
}

void Mutex::unlock()
{
    pthread_mutex_unlock( &m_mu );
}

bool Mutex::testlock()
{
    return true;
}

SharedMutex::SharedMutex()
{
    m_refCount = 1;
}

SharedMutex::~SharedMutex()
{
}

void
SharedMutex::ref()
{
    m_refCount += 1;
}

void
SharedMutex::unref()
{
    m_refCount -= 1;
    if (m_refCount==0) {
        delete this;
    }
}

Monitor::Monitor()
{
    m_mu = new SharedMutex();
    pthread_cond_init(&m_cv, NULL);
}

Monitor::Monitor(Monitor &mon)
{
    mon.m_mu->ref();
    m_mu = mon.m_mu;
    pthread_cond_init(&m_cv, NULL);
}

Monitor::~Monitor()
{
    m_mu->unref();
    pthread_cond_destroy(&m_cv);
}


void
Monitor::enter()
{
    m_mu->lock();
}

void
Monitor::exit()
{
    m_mu->unlock();
}

void
Monitor::signal()
{
    pthread_cond_signal(&m_cv);
}

void
Monitor::wait()
{
    pthread_cond_wait(&m_cv, &m_mu->m_mu);
}
