#include <stdint.h>
#include "malloc.h"
#include "threads.h"
#include "reg.h"

static char keyboard_buffer[MAX_KEYBOARD_BUFFER];
static int  keyboard_buffer_index = 0;
static int  waiting_task_id = -1;

void USART2_attach(int thread_id)
{
	/* TODO:
	 * Need a queue for the future use,
	 * but now we only have shell needs input
	 */
	waiting_task_id = thread_id;
}

void USART2_handler(void)
{
	/* TODO:
	 * Need to implement a mutex here 
	 * prevent critical section from race condition
	 */

	if (keyboard_buffer_index == MAX_KEYBOARD_BUFFER)
		keyboard_buffer[keyboard_buffer_index - 1] = *(USART2_DR) & 0xff;
	else
		keyboard_buffer[keyboard_buffer_index++]   = *(USART2_DR) & 0xff;
	
	/* If there is a task waiting for input, wake it */
	if (waiting_task_id != -1)
		thread_wake(waiting_task_id);
}

char get_input()
{
	return keyboard_buffer[--keyboard_buffer_index];
}
