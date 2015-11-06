#include <lib/x86.h>
#include <lib/thread.h>
#include <lib/spinlock.h>
#include <lib/debug.h>
#include <dev/lapic.h>
#include <pcpu/PCPUIntro/export.h>

#include "import.h"

static spinlock_t thread_lk;
static int cpu_timers[NUM_CPUS]={0};

void thread_spinlock_init(){
	spinlock_init(&thread_lk);
}

void thread_spinlock_acquire(){
	spinlock_acquire(&thread_lk);
}

void thread_spinlock_release(){
	spinlock_release(&thread_lk);
}

void thread_init(unsigned int mbi_addr)
{
	tqueue_init(mbi_addr);
	thread_spinlock_init();
	thread_spinlock_acquire();
	set_curid(0);
	tcb_set_state(0, TSTATE_RUN);
	thread_spinlock_release();
}

/**
 * Allocates new child thread context, set the state of the new child thread
 * as ready, and pushes it to the ready queue.
 * It returns the child thread id.
 */
unsigned int thread_spawn(void *entry, unsigned int id, unsigned int quota)
{
	unsigned int pid;

	thread_spinlock_acquire();
	pid = kctx_new(entry, id, quota);
	tcb_set_state(pid, TSTATE_READY);
	tqueue_enqueue(NUM_IDS, pid);
	thread_spinlock_release();

	return pid;
}

void sched_update(){
	int cur_val;
	cur_val = ++cpu_timers[get_pcpu_idx()];
	if(cur_val >= 5){
		thread_yield();
		cpu_timers[get_pcpu_idx()] = 0;
	}
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
void thread_yield()
{
	unsigned int old_cur_pid;
	unsigned int new_cur_pid;

	thread_spinlock_acquire();
	old_cur_pid = get_curid();
	tcb_set_state(old_cur_pid, TSTATE_READY);
	tqueue_enqueue(NUM_IDS, old_cur_pid);

	new_cur_pid = tqueue_dequeue(NUM_IDS);
	tcb_set_state(new_cur_pid, TSTATE_RUN);
	set_curid(new_cur_pid);

	if (old_cur_pid != new_cur_pid){
		thread_spinlock_release();
		kctx_switch(old_cur_pid, new_cur_pid);
	}
	else{
		thread_spinlock_release();
	}
}
