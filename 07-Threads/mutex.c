#include "threads.h"
#include "malloc.h"

typedef struct {
	int type;
	struct __tcb *owner;
	struct __tcb *queue;
} mutex_t;

typedef *mutex_t mutex_id;

mutex_id mutex_create(int type)
{
	mutex_id mtx = (mutex_id) malloc(sizeof(mutex_t));
	if (mtx) 
		mtx->type = type;
	return mtx;
}
