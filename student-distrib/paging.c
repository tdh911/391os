// this is going to be the paging.c file
#include "paging.h"

/* paging_init
 *
 * DESCRIPTION: Enables paging so that when memory is improperly accessed, the OS
                throws a page fault exception
 *
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: Enables paging
 */
void paging_init(void)
{
    /*
        1) initialize the page directory elements
            a. 4-8MB needs to be one directory with one table
            b. 0-4MB needs to be one directory with 1024 tables **double check with alan

        2) so basically here we need to initialize the 32 bits in each page table in each
           page directory

        3) find a way to fill the space after kernel memory with blank essesntially
    */

    // initialize entire page directory
    int i;
    for(i = 0; i < DIRECTORY_SIZE; i++){
        page_directory[i] = RW;
    }

    // initialize first 4MB in memory at 0
    uint32_t entry = (uint32_t)page_table;
    entry |= RWON;
    page_directory[0] = entry;

    // initialize the kernel memory at 4MB
    entry = KERNEL;
    entry |= SRWON;
    page_directory[1] = entry;

    // fill the page tables so that pages are properly initialized
    for(i = 0; i < DIRECTORY_SIZE; i++){
        page_table[i] = RW;
    }

    // properly handle the video memory segment
    entry = VIDEO;
    entry |= RWON;
    page_table[VIDEO >> 12] = entry;
    page_table[(VIDEO >> 12) + 1] = BACKUP0 | RWON;
    page_table[(VIDEO >> 12) + 2] = BACKUP1 | RWON;
    page_table[(VIDEO >> 12) + 3] = BACKUP2 | RWON;


    // enable paging used the appropriate control registers
    enable_paging();
}

/* enable_paging
 *
 * DESCRIPTION: Enables paging so that when memory is improperly accessed, the OS
                throws a page fault exception
 *
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: Enables paging
 */
void enable_paging()
{
    asm volatile(
                "movl %0, %%eax \n \
                movl %%eax, %%cr3 \n \
                movl %%cr4, %%eax \n \
                orl $0x00000010, %%eax \n \
                movl %%eax, %%cr4 \n \
                movl %%cr0, %%eax \n \
                orl $0x80000001, %%eax \n \
                movl %%eax, %%cr0"
                :
                :"r"(page_directory)
                :"%eax"
                );
}
