/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Name                     :Isaac Styles
// Department Name : Computer and Information Sciences
// File Name                :SafeQueue.h
// Purpose                  :Thread-safe queue template
// Author			        : Isaac Styles, styles@goldmail.etsu.edu
// Create Date	            :Apr 13, 2016
//
//-----------------------------------------------------------------------------------------------------------
//
// Modified Date	: Apr 28, 2016
// Modified By		: Isaac Styles
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef Q_H
#define Q_H
#include <queue>
#include <pthread.h>
#include <semaphore.h>
template <class T>
class SafeQueue
{
    public:
        /// <summary>
        /// Default Constructor intializes mutex and semaphore
        /// </summary>
        SafeQueue<T>()
        {
            pthread_mutex_init( &mutex, NULL );     //initialize mutex
            sem_init(&sem,0,0);                     //initialize semaphore
        }
        /// <summary>
        /// Push to stack and Post onto the semaphore
        /// </summary>
        void enqueue(T& p)
        {
            pthread_mutex_lock(&mutex);
            vals.push(p);
            pthread_mutex_unlock(&mutex);
            sem_post(&sem);
        }
        /// <summary>
        /// Wait on queue and Pop from stack
        /// </summary>
        T dequeue()
        {
            T p;
            sem_wait(&sem);
            pthread_mutex_lock(&mutex);
            p = vals.front();
            vals.pop();
            pthread_mutex_unlock(&mutex);
            return p;
        }
        /// <summary>
        /// Return true if queue is empty
        /// </summary>
        bool isEmpty()
        {
            if (vals.size() == 0)
            {
                return true;
            }
            return false;
        }
    protected:
    private:
        queue<T> vals;                          //standard queue<T>
        pthread_mutex_t mutex;                  //protects the queue with atomic access
        sem_t sem;                              //wait queue
};

#endif // Q_H
