#ifndef SYS_HANDLER_H_H
#define SYS_HANDLER_H_H

#include "sys_handlers.h"
#include "lib.h"
#include "fs.h"
#include "paging.h"
#include "rtc.h"
#include "terminal.h"
#include "keyboard.h"
#include "i8259.h"

#define SHELL0 0
#define SHELL1 1
#define SHELL2 2
#define NUM_TERMINALS 3
#define USER_PROG 32
#define PG_SIZE 4096


void init_kernel_memory();
void init_shell();
void switch_terminal(int32_t shell);
void demote_pcb_backup(int32_t shell,process_control_block_t* pcb);

#endif
