#ifndef IDT_H
#define IDT_H


#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "schedule.h"
#include "idt_wrappers.h"
#include "sys_handlers.h"

#define SYS_CALL 0x80
#define RTC 0x28
#define KEYBOARD 0x21
#define PIT 0x20
#define NUM_SYS_HANDLERS 20
#define RESERVED 15

#define RING0 0
#define RING1 1
#define RING2 2
#define RING3 3

#define READY 1
#define NOT 0

extern void init_idt();



void exception_handler();

void divide_handler();

void debug_handler();

void nmi_interrupt_handler();

void breakpoint_handler();

void overflow_handler();

void bound_range_handler();

void invalid_opcode_handler();

void device_not_avail_handler();

void dbl_fault_handler();

void coprocess_seg_handler();

void inval_tss_handler();

void seg_not_pres_handler();

void stack_fault_handler();

void gen_protect_handler();

void page_fault_handler();

void float_point_handler();

void align_check_handler();

void machine_check_handler();

void simd_float_point_handler();

#endif
