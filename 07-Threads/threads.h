#ifndef __THREADS_H__
#define __THREADS_H__

/* Thread Control Block */
typedef struct {
	void *stack;
	void *orig_stack;
	uint8_t state;
} tcb_t;

enum {
	THREAD_NONE,
	THREAD_ACTIVE,
	THREAD_SLEEP
};

void thread_start();
int thread_create(void (*run)(void*), void* userdata);
void thread_kill(int thread_id);
void thread_self_terminal();
void thread_wake(int thread_id);
void thread_sleep(int thread_id);
int get_thread_id();

#endif
