#ifndef __GM_LOCKER_H__
#define __GM_LOCKER_H__

#include <pthread.h>
#include <errno.h>

class CGMSafePLock
{
private:
    pthread_mutex_t *m_pMutex;

public:
    CGMSafePLock(pthread_mutex_t *plock)
    {
        m_pMutex = plock;
        if(m_pMutex)
        {
            pthread_mutex_lock(m_pMutex);
        }
    }

    ~CGMSafePLock()
    {
        if(m_pMutex)
        {
            pthread_mutex_unlock(m_pMutex);
        }
    }
};

class CGMSafeAntiPLock
{
private:
    pthread_mutex_t *m_pMutex;

public:
    CGMSafeAntiPLock(pthread_mutex_t *plock)
    {
        m_pMutex = plock;
        if(m_pMutex)
        {
            pthread_mutex_unlock(m_pMutex);
        }
    }

    ~CGMSafeAntiPLock()
    {
        if(m_pMutex)
        {
            pthread_mutex_lock(m_pMutex);
        }
    }
};

class CGMSafeRWLock
{
private:
    pthread_rwlock_t * m_pRwLock;

public:
    CGMSafeRWLock(pthread_rwlock_t * pRwLock, bool bWriteLock)
    {
        m_pRwLock = pRwLock;
        if(m_pRwLock)
        {
            if (bWriteLock)
            {
                pthread_rwlock_wrlock(m_pRwLock);
            }
            else
            {
                pthread_rwlock_rdlock(m_pRwLock);
            }
        }
    }

    ~CGMSafeRWLock()
    {
        if(m_pRwLock)
        {
            pthread_rwlock_unlock(m_pRwLock);
        }
    }
};

class CGMNotifyEvent
{
private:
    pthread_mutex_t    lk;
    pthread_cond_t      cd;

    //0 没有事件; 1 有事件了
    int                         flag;
public:
    enum WAIT_TIME
    {
        WAIT_INFINITE = 0xFFFFFFFF,
    };
    enum WAIT_RETURN
    {
        RETURN_EVENT = 0,
        RETURN_TIMEOUT = 1,
        RETURN_FAILD = 0xFFFFFFFF,
    };

    CGMNotifyEvent()
    {
        pthread_mutex_init(&lk, NULL);
        pthread_cond_init(&cd, NULL);
        flag = 0;
    }
    ~CGMNotifyEvent()
    {
        pthread_mutex_destroy(&lk);
        pthread_cond_destroy(&cd);
    }

    void Notify()
    {
        pthread_mutex_lock(&lk);
        flag = 1;
        pthread_cond_signal(&cd);
        pthread_mutex_unlock(&lk);
    }

    void Reset()
    {
        pthread_mutex_lock(&lk);
        flag = 0;
        pthread_mutex_unlock(&lk);
    }

    int WaitEvent(unsigned long milliseconds)
    {
        int ret;
        if (milliseconds == WAIT_INFINITE)
        {
            ret = pthread_mutex_lock(&lk);
            if(flag == 1)
            {
                flag = 0;
                ret = pthread_mutex_unlock(&lk);
                return (int)RETURN_EVENT;
            }

            ret = pthread_cond_wait(&cd, &lk);
            if(flag == 1)
            {
                flag = 0;
                ret = pthread_mutex_unlock(&lk);
                return (int)RETURN_EVENT;
            }
            
            ret = pthread_mutex_unlock(&lk);
            return (int)RETURN_FAILD;
        }
        else //or milliseconds
        {
            pthread_mutex_lock(&lk);
            if(flag == 1)
            {
                flag = 0;
                ret = pthread_mutex_unlock(&lk);
                return (int)RETURN_EVENT;
            }

            struct timespec nxttime;
            clock_gettime(CLOCK_REALTIME,&nxttime);
            nxttime.tv_sec += milliseconds / 1000;
            nxttime.tv_nsec += (milliseconds % 1000) * 1000000;

            while (nxttime.tv_nsec >= 1000000000)
            {
                nxttime.tv_sec++;
                nxttime.tv_nsec -= 1000000000;
            }

            ret = pthread_cond_timedwait(&cd, &lk, &nxttime);
            if(flag == 1)
            {
                flag = 0;
                ret = pthread_mutex_unlock(&lk);
                return (int)RETURN_EVENT;
            }

            pthread_mutex_unlock(&lk);

            //退出
            switch(ret)
            {
            case ETIMEDOUT:
                return (int)RETURN_TIMEOUT;
            default:
                return (int)RETURN_FAILD;
            }
        }
    }
};


class CGMBroadCastEvent
{
private:
    pthread_mutex_t    lk;
    pthread_cond_t      cd;
public:
    CGMBroadCastEvent()
    {
        pthread_mutex_init(&lk, NULL);
        pthread_cond_init(&cd, NULL);
    }
    ~CGMBroadCastEvent()
    {
        pthread_mutex_destroy(&lk);
        pthread_cond_destroy(&cd);
    }

    void Notify()
    {
        pthread_mutex_lock(&lk);
        pthread_cond_broadcast(&cd);
        pthread_mutex_unlock(&lk);
    }

    int WaitEvent(unsigned long milliseconds)
    {
        pthread_mutex_lock(&lk);
        struct timespec nxttime;
        clock_gettime(CLOCK_REALTIME,&nxttime);
        nxttime.tv_sec += milliseconds / 1000;
        nxttime.tv_nsec += (milliseconds % 1000) * 1000000;

        while (nxttime.tv_nsec >= 1000000000)
        {
            nxttime.tv_sec++;
            nxttime.tv_nsec -= 1000000000;
        }

        int ret = pthread_cond_timedwait(&cd, &lk, &nxttime);
        pthread_mutex_unlock(&lk);
        return ret;
    }
};


#endif

