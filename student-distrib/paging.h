// this is going to be the paging.h file
#ifndef PAGING_H
#define PAGING_H

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"

#define DIRECTORY_SIZE 1024
#define PAGE_SIZE (DIRECTORY_SIZE * 4)
#define VIDEO 0xB8000
#define BACKUP0 0xB9000
#define BACKUP1 0xBA000
#define BACKUP2 0xBB000
#define KERNEL 0x400000

#define RW 0x02
#define RWON 0x03
#define URWON 0x07
#define SRWON 0x83
#define SURWON 0x87

uint32_t page_directory[DIRECTORY_SIZE] __attribute__((aligned (PAGE_SIZE)));
uint32_t page_table[DIRECTORY_SIZE] __attribute__((aligned (PAGE_SIZE)));
uint32_t page_table_vid[DIRECTORY_SIZE] __attribute__((aligned (PAGE_SIZE)));

extern void paging_init(void);
extern void enable_paging();

#endif
