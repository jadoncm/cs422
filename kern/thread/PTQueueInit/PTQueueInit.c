#include "lib/x86.h"

#include "import.h"

/**
 * Initializes all the thread queues with
 * tqueue_init_at_id.
 */
void tqueue_init(unsigned int mbi_addr)
{
	int i;
	tcb_init(mbi_addr);
	
	for(i = 0; i <= NUM_IDS; i++){
		tqueue_init_at_id(i);
	}
}

/**
 * Insert the TCB #pid into the tail of the thread queue #chid.
 * Recall that the doubly linked list is index based.
 * So you only need to insert the index.
 * Hint: there are multiple cases in this function.
 */
void tqueue_enqueue(unsigned int chid, unsigned int pid)
{
	unsigned int head, tail;
	head = tqueue_get_head(chid);
	tail = tqueue_get_tail(chid);
  	if(head == NUM_IDS && tail == NUM_IDS){
		tqueue_set_tail(chid, pid);
		tcb_set_next(pid, NUM_IDS);
		tcb_set_prev(pid, NUM_IDS);
	}
	else if(head == NUM_IDS && tail != NUM_IDS){
		tqueue_set_tail(chid, pid);
		tqueue_set_head(chid, tail);
		tcb_set_next(tail, pid);
		tcb_set_prev(tail, NUM_IDS);
		tcb_set_prev(pid, tail);
		tcb_set_next(pid, NUM_IDS);
	}
	else if(head != NUM_IDS && tail == NUM_IDS){
		tqueue_set_tail(chid, pid);
		tcb_set_next(head, pid);
		tcb_set_prev(pid, head);
		tcb_set_next(pid, NUM_IDS);
	}
	else{
		tcb_set_next(tail, pid);
		tcb_set_prev(pid, tail);
		tcb_set_next(pid, NUM_IDS);
		tqueue_set_tail(chid, pid);
	}
}

/**
 * Reverse action of tqueue_enqueue, i.g., pops a TCB from the head of specified queue.
 * It returns the poped thread's id, or NUM_IDS if the queue is empty.
 * Hint: there are mutiple cases in this function.
 */
unsigned int tqueue_dequeue(unsigned int chid)
{
	unsigned int next, head, tail;
	head = tqueue_get_head(chid);
	tail = tqueue_get_tail(chid);
  	if(head == NUM_IDS && tail == NUM_IDS){
		return NUM_IDS;
	}
	else if(head == NUM_IDS && tail != NUM_IDS){
		tcb_set_prev(tail, NUM_IDS);
		tcb_set_next(tail, NUM_IDS);
		tqueue_set_tail(chid, NUM_IDS);
		return tail;
	}
	else{
		next = tcb_get_next(head);
		tcb_set_prev(head, NUM_IDS);
		tcb_set_next(head, NUM_IDS);
		tqueue_set_head(chid, next);
		if(next == NUM_IDS) return head;
		tcb_set_prev(next, NUM_IDS);
		return head;
	}
}

/**
 * Removes the TCB #pid from the queue #chid.
 * Hint: there are many cases in this function.
 */
void tqueue_remove(unsigned int chid, unsigned int pid)
{
	unsigned int head, tail, prev, next;
	head = tqueue_get_head(chid);
	tail = tqueue_get_tail(chid);
	if(head == pid){
		tqueue_dequeue(chid);
	}
	else if(tail == pid){
		prev = tcb_get_prev(pid);
		tqueue_set_tail(chid, prev);
		tcb_set_prev(pid, NUM_IDS);
		tcb_set_next(pid, NUM_IDS);
		if(prev == NUM_IDS) return;
		tcb_set_next(prev, NUM_IDS);
	}
	else{
		prev = tcb_get_prev(pid);
		next = tcb_get_next(pid);
		tcb_set_next(prev, next);
		tcb_set_prev(next, prev);
		tcb_set_prev(pid, NUM_IDS);
		tcb_set_next(pid, NUM_IDS);
	}
}
