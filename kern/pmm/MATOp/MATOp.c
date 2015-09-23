#include <lib/debug.h>
#include "import.h"

#define PAGESIZE      4096
#define VM_USERLO     0x40000000
#define VM_USERHI     0xF0000000
#define VM_USERLO_PI  (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI  (VM_USERHI / PAGESIZE)

/**
 * Allocation of a physical page.
 *
 * 1. First, implement a naive page allocator that scans the allocation table (AT) 
 *    using the functions defined in import.h to find the first unallocated page
 *    with usable permission.
 *    (Q: Do you have to scan allocation table from index 0? Recall how you have
 *    initialized the table in pmem_init.)
 *    Then mark the page as allocated in the allocation table and return the page
 *    index of the page found. In the case when there is no avaiable page found,
 *    return 0.
 * 2. Optimize the code with the memorization techniques so that you do not have to
 *    scan the allocation table from scratch every time.
 */

unsigned int last_page = VM_USERLO_PI; //stores the last page searched (memoization)

unsigned int
palloc()
{
  unsigned int start_page;
  start_page = last_page;

  do{ 
    if(last_page >= VM_USERHI_PI) last_page = VM_USERLO_PI;

    if(at_is_norm(last_page) && !at_is_allocated(last_page))
    {
      at_set_allocated(last_page, 1);
      return last_page;
    }
 
    last_page++;
  }while(start_page != last_page);

  return 0;
} 


/**
 * Free of a physical page.
 *
 * This function marks the page with given index as unallocated
 * in the allocation table.
 *
 * Hint: Simple.
 */
void
pfree(unsigned int pfree_index)
{
  at_set_allocated(pfree_index, 0);
}
