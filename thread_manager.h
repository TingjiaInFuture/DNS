#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

void thread_manager_init(int num_threads);
void thread_manager_destroy();
void thread_manager_add_task(void (*task_function)(void*, int), void* arg, int detailedDebug);

#endif // THREAD_MANAGER_H
