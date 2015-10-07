#include <lib/x86.h>
#include <lib/debug.h>
#include "import.h"

#define VM_USERLO   0x40000000
#define VM_USERHI   0xF0000000

#define get_pde(vaddr) vaddr >> 22
#define get_pte(vaddr) (vaddr >> 12) & ((1 << 10) - 1)

/**
 * Returns the page table entry corresponding to the virtual address,
 * according to the page structure of process # [proc_index].
 * Returns 0 if the mapping does not exist.
 */
unsigned int get_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
     if (get_pdir_entry(proc_index,get_pde(vaddr)))
        return get_ptbl_entry(proc_index,get_pde(vaddr),get_pte(vaddr));
    return 0;
}         

// returns the page directory entry corresponding to the given virtual address
unsigned int get_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    return get_pdir_entry(proc_index, get_pde(vaddr));
}

// removes the page table entry for the given virtual address
void rmv_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    rmv_ptbl_entry(proc_index,get_pde(vaddr),get_pte(vaddr));
}

// removes the page directory entry for the given virtual address
void rmv_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    rmv_pdir_entry(proc_index,get_pde(vaddr));
}

// maps the virtual address [vaddr] to the physical page # [page_index] with permission [perm]
// you do not need to worry about the page directory entry. just map the page table entry.
void set_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr, unsigned int page_index, unsigned int perm)
{
    set_ptbl_entry(proc_index,get_pde(vaddr),get_pte(vaddr),page_index,perm);
}

// registers the mapping from [vaddr] to physical page # [page_index] in the page directory
void set_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr, unsigned int page_index)
{
    set_pdir_entry(proc_index,get_pde(vaddr),page_index);
}   

// initializes the identity page table
// the permission for the kernel memory should be PTE_P, PTE_W, and PTE_G,
// while the permission for the rest should be PTE_P and PTE_W.
void idptbl_init(unsigned int mbi_adr)
{
    container_init(mbi_adr);

    int i,j;
    for (i = 0; i < 1024; ++i) { 
        for (j = 0; j < 1024; ++j) {
            unsigned int addr = (i << 22) | (j << 12);
            if (addr < VM_USERLO || addr >= VM_USERHI) {
                set_ptbl_entry_identity(i,j, PTE_P | PTE_W | PTE_G);
            }
            else {
                set_ptbl_entry_identity(i,j,PTE_P | PTE_W);
            }
        }
    }
}
