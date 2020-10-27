#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*
* A typical structure for a read write lock instance.
* reader_count: Keeps track of the number of readers within the code block.
* writer: A flag used to determine whether a writer is currently writing.
* mutex: A simple mutex.
* read_wait: Condition variable on which a reader waits if there is a writer.
* write_wait:  Condition variable on which a writer waits if there is a reader.
*/
struct rwlock
{
    int reader_count, writer;
    pthread_mutex_t mutex;
    pthread_cond_t read_wait;
    pthread_cond_t write_wait;
};

/*
* Below are the prototype functions that are required for implementing a reader-writer lock.
*/
static void *init_rwlock(struct rwlock *);
static void read_lock(struct rwlock *);
static void read_unlock(struct rwlock *);
static void write_lock(struct rwlock *);
static void write_unlock(struct rwlock *);

/*
* This function will initialise all the variables mentioned in the struct rwlock.
*/
static void *init_rwlock (struct rwlock *rwl) {
    rwl->reader_count = 0;
    rwl->writer=0;
    pthread_mutex_init(&rwl->mutex, NULL);
    pthread_cond_init(&rwl->read_wait, NULL);
    pthread_cond_init(&rwl->write_wait, NULL);
    return rwl;
}

/*
* This function is for when the program wants to read a particular resource.
* Incase there is a writer, the reader will wait for the writer to finish.
* Else it enters the section which it will read and increments the reader_count
*/
static void read_lock (struct rwlock *rwl) {
    pthread_mutex_lock(&rwl->mutex);
    while (rwl->writer)
    {
        pthread_cond_wait(&rwl->read_wait, &rwl->mutex);
    }
    rwl->reader_count++;
    pthread_mutex_unlock(&rwl->mutex);
}

/*
* This function is for when the program has finished reading the resource.
* It will decrement the reader_count by 1.
* Incase reader_count==0, then it will signal all the writers that might be waiting.
*/
static void read_unlock (struct rwlock *rwl) {
    pthread_mutex_lock(&rwl->mutex);
    rwl->reader_count--;
    if(rwl->reader_count==0){
        pthread_cond_broadcast(&rwl->write_wait);
    }
    pthread_mutex_unlock(&rwl->mutex);
}

/*
* This function is for when the program wants to write a particular resource.
* Incase there is a reader, the writer will wait for the reader to finish.
* It will set the writer flag to 1.
*/
static void write_lock (struct rwlock *rwl) {
    pthread_mutex_lock(&rwl->mutex);
    while (rwl->reader_count>0 || rwl->writer)
    {
        pthread_cond_wait(&rwl->write_wait, &rwl->mutex);
    }
    rwl->writer=1;
    pthread_mutex_unlock(&rwl->mutex);
}

/*
* This function is for when the program has finished writing a particular resource.
* It then broadcasts all the readers waiting on this resource
* It will set the writer flag to 0.
*/
static void write_unlock (struct rwlock *rwl) {
    pthread_mutex_lock(&rwl->mutex);
    rwl->writer=0;
    pthread_cond_broadcast(&rwl->read_wait);
    pthread_mutex_unlock(&rwl->mutex);
}