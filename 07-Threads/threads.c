#include <stdint.h>
#include "threads.h"
#include "os.h"
#include "malloc.h"
#include "reg.h"

#define THREAD_PSP	0xFFFFFFFD

/* Thread Control Block */
typedef struct {
	void *stack;
	void *orig_stack;
	/* 0 -> no, 1 -> active, 2 -> sleep */
	uint8_t state;
} tcb_t;

static tcb_t tasks[MAX_TASKS];
static int lastTask;
static int first = 1;
static int active_task_num = 0;

/* FIXME: Without naked attribute, GCC will corrupt r7 which is used for stack
 * pointer. If so, after restoring the tasks' context, we will get wrong stack
 * pointer.
 */
void __attribute__((naked)) pendsv_handler()
{
	/* Save the old task's context */
	asm volatile("mrs   r0, psp\n"
	             "stmdb r0!, {r4-r11, lr}\n"
	             "mov   %0, r0\n"
		     : "=r" (tasks[lastTask].stack));

	/* Run idle task if there is no other task */
	if (active_task_num == 1) {
		lastTask = 0;
		asm volatile("mov r0, %0\n"
		             "ldmia r0!, {r4-r11, lr}\n"
		             "msr psp, r0\n"
		             "bx lr\n"
		             : : "r" (tasks[0].stack));
	}

	else {
		/* Find a new task to run, except idle task */
		while (1) {
			lastTask++;
			if (lastTask == MAX_TASKS)
				lastTask = 1;
			if (tasks[lastTask].state == 1) {
				/* Restore the new task's context and jump to the task */
				asm volatile("mov r0, %0\n"
				             "ldmia r0!, {r4-r11, lr}\n"
				             "msr psp, r0\n"
				             "bx lr\n"
				             : : "r" (tasks[lastTask].stack));
			}
		}
	}
}

void systick_handler()
{
	*SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void thread_start()
{
	lastTask = 1; // reserve index 0 for idle task

	/* Save kernel context */
	asm volatile("mrs ip, psr\n"
	             "push {r4-r11, ip, lr}\n");

	/* Load user task's context and jump to the task */
	asm volatile("mov r0, %0\n"
	             "msr psp, r0\n"
	             "mov r0, #3\n"
	             "msr control, r0\n"
	             "isb\n"
	             "pop {r4-r11, lr}\n"
	             "pop {r0}\n"
	             "bx lr\n"
	             : : "r" (tasks[lastTask].stack));
}

int thread_create(void (*run)(void *), void *userdata)
{
	/* Find a free thing */
	int threadId = 0;
	uint32_t *stack;

	for (threadId = 0; threadId < MAX_TASKS; threadId++) {
		if (tasks[threadId].state == 0)
			break;
	}

	if (threadId == MAX_TASKS)
		return -1;

	/* Create the stack */
	stack = (uint32_t *) malloc(STACK_SIZE * sizeof(uint32_t));
	tasks[threadId].orig_stack = stack;
	if (stack == 0)
		return -1;

	stack += STACK_SIZE - 32; /* End of stack, minus what we are about to push */

	/* For thread_start(), reserve 0 for idle task */
	if (threadId == 1 && first == 1) {
		first = 0;
		stack[8] = (unsigned int) run;
		stack[9] = (unsigned int) userdata;
	} else {
		stack[8] = (unsigned int) THREAD_PSP;
		stack[9] = (unsigned int) userdata;
		stack[14] = (unsigned) &thread_self_terminal;
		stack[15] = (unsigned int) run;
		stack[16] = (unsigned int) 0x21000000; /* PSR Thumb bit */
	}

	/* Construct the control block */
	tasks[threadId].stack = stack;
	tasks[threadId].state = 1;
	active_task_num++;

	return threadId;
}

void thread_kill(int thread_id)
{
	if (tasks[thread_id].state == 1)
		active_task_num--;

	tasks[thread_id].state = 0;

	/* Free the stack */
	free(tasks[thread_id].orig_stack);
}

void thread_self_terminal()
{
	/* This will kill the stack.
	 * For now, disable context switches to save ourselves.
	 */

	/* It's like the lock , which can prevent this critical
	 * from being interrupt. */
	asm volatile("cpsid i\n");
	thread_kill(lastTask);
	asm volatile("cpsie i\n");

	/* And now wait for death to kick in */
	while (1);
}

void thread_wake(int thread_id)
{
	if (tasks[thread_id].state == 2)
		active_task_num++;
		
	tasks[thread_id].state = 1;
	lastTask = thread_id - 1;
	*SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void thread_sleep(int thread_id)
{
	if (tasks[thread_id].state == 1)
		active_task_num--;

	tasks[thread_id].state = 2;
	
	/* find next task */
	*SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

int get_thread_id()
{
	asm volatile("cpsid i\n");
	int id = lastTask;
	asm volatile("cpsie i\n");
	return id;
}
