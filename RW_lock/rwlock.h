#include<stdio.h>
#include<pthread.h>

struct rwlock
{
    int reader_count, writer;
    pthread_mutex_t mutex;
    pthread_cond_t read_wait;
    pthread_cond_t write_wait;
};

static void init_rwlock(struct rwlock *);
static void read_lock(struct rwlock *);
static void read_unlock(struct rwlock *);
static void write_lock(struct rwlock *);
static void write_unlock(struct rwlock *);


static void init_rwlock(struct rwlock *rwl){
    rwl->reader_count = 0;
    rwl->writer=0;
    pthread_mutex_init(&rwl->mutex, NULL);
    pthread_cond_init(&rwl->read_wait, NULL);
    pthread_cond_init(&rwl->write_wait, NULL);
}

static void read_lock(struct rwlock *rwl){
    pthread_mutex_lock(&rwl->mutex);
    while (rwl->writer)
    {
        pthread_cond_wait(&rwl->read_wait, &rwl->mutex);
    }
    rwl->reader_count++;
    pthread_mutex_unlock(&rwl->mutex);
}

static void read_unlock(struct rwlock *rwl){
    pthread_mutex_lock(&rwl->mutex);
    rwl->reader_count--;
    if(rwl->reader_count==0){
        pthread_cond_broadcast(&rwl->write_wait);
    }
    pthread_mutex_unlock(&rwl->mutex);
}

static void write_lock(struct rwlock *rwl){
    pthread_mutex_lock(&rwl->mutex);
    while (rwl->reader_count>0 || rwl->writer)
    {
        pthread_cond_wait(&rwl->write_wait, &rwl->mutex);
    }
    rwl->writer=1;
    pthread_mutex_unlock(&rwl->mutex);
}

static void write_unlock(struct rwlock *rwl){
    pthread_mutex_lock(&rwl->mutex);
    rwl->writer=0;
    pthread_cond_broadcast(&rwl->read_wait);
    pthread_mutex_unlock(&rwl->mutex);
}