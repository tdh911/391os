//this file is for scheduling heaader file
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "x86_desc.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "sys_handlers.h"

#define PIT_0_DATA_PORT 0x40
#define MODE_COMMAND_REG 0x43
#define PIT_IRQ_NUM 0

#define PIT_MODE_3 0x36
#define DIV_100HZ 1193180/100
#define MASK_FREQ 0xFF
#define SCHED_SIZE 3

int32_t curr;

extern void pit_init(void);
extern void pit_handler();



#endif
