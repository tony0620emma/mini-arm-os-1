#ifndef __THREADS_H__
#define __THREADS_H__

void thread_start();
int thread_create(void (*run)(void*), void* userdata);
void thread_kill(int thread_id);
void thread_self_terminal();
void thread_wake(int thread_id);
void thread_sleep();
int get_thread_id();

#endif
