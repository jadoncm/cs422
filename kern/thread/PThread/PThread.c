#include <lib/x86.h>
#include <lib/thread.h>

#include "import.h"

void thread_init(unsigned int mbi_addr)
{
	tqueue_init(mbi_addr);
	set_curid(0);
	tcb_set_state(0, TSTATE_RUN);
}

/**
 * Allocates new child thread context, set the state of the new child thread
 * as ready, and pushes it to the ready queue.
 * It returns the child thread id.
 */
unsigned int thread_spawn(void *entry, unsigned int id, unsigned int quota)
{
	unsigned int pid;
	pid = kctx_new(entry, id, quota);
	tcb_set_state(pid, TSTATE_READY);
	tqueue_enqueue(NUM_IDS, pid);
	return pid;
}

/**
 * Yield to the next thread in the ready queue.
 * You should set the currently running thread state as ready,
 * and push it back to the ready queue.
 * And set the state of the poped thread as running, set the
 * current thread id, then switches to the new kernel context.
 * Hint: if you are the only thread that is ready to run,
 * do you need to switch to yourself?
 */
void thread_yield(void)
{
	unsigned int cur, next;
	cur = get_curid();
	next = tqueue_dequeue(NUM_IDS);
	
	if(cur == next){
		tqueue_enqueue(NUM_IDS, cur);
		return;	
	}
	if(next == NUM_IDS){
		return;
	}
	
	tcb_set_state(cur, TSTATE_READY);
	tqueue_enqueue(NUM_IDS, cur);
	
	tcb_set_state(next, TSTATE_RUN);	
	set_curid(next);
	kctx_switch(cur, next);
}
