#include "thread_manager.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include "platform.h"

#ifdef _WIN32
typedef struct {
    void (*function)(void *);
    void *arg;
} task_t;

typedef struct {
    HANDLE *threads;
    HANDLE mutex;
    HANDLE event;
    task_t *task_queue;
    int task_count;
    int stop;
    int queue_size;
    int queue_front;
    int queue_rear;
} thread_pool_t;

static thread_pool_t *pool = NULL;
static int num_threads;

DWORD WINAPI worker_thread(LPVOID arg) {
    while (1) {
        WaitForSingleObject(pool->event, INFINITE);
        WaitForSingleObject(pool->mutex, INFINITE);

        if (pool->stop && pool->task_count == 0) {
            ReleaseMutex(pool->mutex);
            break;
        }

        if (pool->task_count > 0) {
            task_t task = pool->task_queue[pool->queue_front];
            pool->queue_front = (pool->queue_front + 1) % pool->queue_size;
            pool->task_count--;

            if (pool->task_count > 0) {
                SetEvent(pool->event);
            } else {
                ResetEvent(pool->event);
            }

            ReleaseMutex(pool->mutex);
            task.function(task.arg);
        } else {
            ReleaseMutex(pool->mutex);
        }
    }

    return 0;
}

void thread_manager_init(int n) {
    num_threads = n;
    pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (!pool) {
        log_error("Failed to allocate memory for thread pool");
        exit(EXIT_FAILURE);
    }

    pool->task_queue = (task_t *)malloc(sizeof(task_t) * 1024);
    pool->queue_size = 1024;
    pool->queue_front = 0;
    pool->queue_rear = 0;
    pool->task_count = 0;
    pool->stop = 0;

    pool->mutex = CreateMutex(NULL, FALSE, NULL);
    pool->event = CreateEvent(NULL, TRUE, FALSE, NULL);

    pool->threads = (HANDLE *)malloc(sizeof(HANDLE) * num_threads);

    for (int i = 0; i < num_threads; i++) {
        pool->threads[i] = CreateThread(NULL, 0, worker_thread, NULL, 0, NULL);
    }
}

void thread_manager_destroy() {
    WaitForSingleObject(pool->mutex, INFINITE);
    pool->stop = 1;
    SetEvent(pool->event);
    ReleaseMutex(pool->mutex);

    for (int i = 0; i < num_threads; i++) {
        WaitForSingleObject(pool->threads[i], INFINITE);
        CloseHandle(pool->threads[i]);
    }

    CloseHandle(pool->mutex);
    CloseHandle(pool->event);
    free(pool->task_queue);
    free(pool->threads);
    free(pool);
}

void thread_manager_add_task(void (*task_function)(void *), void *arg) {
    WaitForSingleObject(pool->mutex, INFINITE);

    pool->task_queue[pool->queue_rear].function = task_function;
    pool->task_queue[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_size;
    pool->task_count++;

    SetEvent(pool->event);
    ReleaseMutex(pool->mutex);
}

#else // POSIX implementation

#include <pthread.h>

typedef struct {
    void (*function)(void *);
    void *arg;
} task_t;

typedef struct {
    pthread_t *threads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    task_t *task_queue;
    int task_count;
    int stop;
    int queue_size;
    int queue_front;
    int queue_rear;
} thread_pool_t;

static thread_pool_t *pool = NULL;
static int num_threads;

void *worker_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&pool->mutex);

        while (pool->task_count == 0 && !pool->stop) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }

        if (pool->stop && pool->task_count == 0) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        if (pool->task_count > 0) {
            task_t task = pool->task_queue[pool->queue_front];
            pool->queue_front = (pool->queue_front + 1) % pool->queue_size;
            pool->task_count--;

            pthread_cond_signal(&pool->cond);
            pthread_mutex_unlock(&pool->mutex);

            task.function(task.arg);
        } else {
            pthread_mutex_unlock(&pool->mutex);
        }
    }

    return NULL;
}

void thread_manager_init(int n) {
    num_threads = n;
    pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (!pool) {
        log_error("Failed to allocate memory for thread pool");
        exit(EXIT_FAILURE);
    }

    pool->task_queue = (task_t *)malloc(sizeof(task_t) * 1024);
    pool->queue_size = 1024;
    pool->queue_front = 0;
    pool->queue_rear = 0;
    pool->task_count = 0;
    pool->stop = 0;

    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, worker_thread, NULL);
    }
}

void thread_manager_destroy() {
    pthread_mutex_lock(&pool->mutex);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);

    for (int i = 0; i < num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    free(pool->task_queue);
    free(pool->threads);
    free(pool);
}

void thread_manager_add_task(void (*task_function)(void *), void *arg) {
    pthread_mutex_lock(&pool->mutex);

    pool->task_queue[pool->queue_rear].function = task_function;
    pool->task_queue[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_size;
    pool->task_count++;

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
}
#endif
