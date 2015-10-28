#include <lib/x86.h>

#include "import.h"

/**
 * Initializes the TCB for all NUM_IDS threads with the
 * state TSTATE_DEAD, and with two indices being NUM_IDS (which represents NULL).
 */
void tcb_init(unsigned int mbi_addr)
{
	int i;
	paging_init(mbi_addr);
	
	for(i = 0; i < NUM_IDS; i++){
		tcb_init_at_id(i);
	} 
  
}
