#include "idt.h"

//static array of all system handlers
static uint32_t sys_handlers[NUM_SYS_HANDLERS] = {
    (uint32_t)divide_handler,
    (uint32_t)debug_handler,
    (uint32_t)nmi_interrupt_handler,
    (uint32_t)breakpoint_handler,
    (uint32_t)overflow_handler,
    (uint32_t)bound_range_handler,
    (uint32_t)invalid_opcode_handler,
    (uint32_t)device_not_avail_handler,
    (uint32_t)dbl_fault_handler,
    (uint32_t)coprocess_seg_handler,
    (uint32_t)inval_tss_handler,
    (uint32_t)seg_not_pres_handler,
    (uint32_t)stack_fault_handler,
    (uint32_t)gen_protect_handler,
    (uint32_t)page_fault_handler,
    (uint32_t)exception_handler,//general exception here
    (uint32_t)float_point_handler,
    (uint32_t)align_check_handler,
    (uint32_t)machine_check_handler,
    (uint32_t)simd_float_point_handler
};


/* init_idt
 *
 * DESCRIPTION: Initializes IDT by looping throught table and setting
 *              corresponding descriptor bits. Also sets offsets to
 *              for various entries in the table.
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: fills IDT with corresponding descriptor values and offsets
 */
void init_idt(){

  //load the pointer to idt into the IDTR
  lidt(idt_desc_ptr);

  int i;
  //loop through each vector in the idt and set the corresponding
  //bits in the descriptor
  for(i=0;i<NUM_VEC;i++){

    idt[i].seg_selector = KERNEL_CS;
    //priority level should be 0 for non system calls
    idt[i].dpl = RING0;

    idt[i].size = 1;
    idt[i].present = 1;

    //mark every descriptor as 01110 in bit values to signify interrupt gate
    idt[i].reserved0 = 0;
    idt[i].reserved1 = 1;
    idt[i].reserved2 = 1;
    idt[i].reserved3 = 1;//was 0
    idt[i].reserved4 = 0;

    //if system call, priority level is 3
    if(i == SYS_CALL)
      idt[i].dpl = RING3;

    //assign correct handler
    if(i < NUM_SYS_HANDLERS && i != RESERVED)
        SET_IDT_ENTRY(idt[i],sys_handlers[i]);
    else if(i == SYS_CALL)
        SET_IDT_ENTRY(idt[i],system_handler_wrapper);
    else if(i == KEYBOARD)
        SET_IDT_ENTRY(idt[i],keyboard_handler_wrapper);
    else if(i == RTC)
        SET_IDT_ENTRY(idt[i],rtc_handler_wrapper);
    else if(i == PIT)
        SET_IDT_ENTRY(idt[i],pit_handler_wrapper);
    else{
        idt[i].present = 0;
        SET_IDT_ENTRY(idt[i],exception_handler);
    }

  }

}



/* exception_handler
 *
 * DESCRIPTION: General exception handler
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void exception_handler(){
    printf("Unknown error occured");
    while(1);
}

/* divide_handler
 *
 * DESCRIPTION: Handler for Divide Error Exception (#DE)
                Called upon a divide by 0 or result would overflow
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void divide_handler(){
    clear();
    printf("Interrupt 0 - Divide Error Exception\n");
    while(1);
}

/* debug_handler
 *
 * DESCRIPTION: A debug exception has been caught
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void debug_handler(){
    printf("Interrupt 1 - Debug Exception\n");
    system_handler(SYS_HALT,256,0,0);
}

/* nmi_interrupt_handler
 *
 * DESCRIPTION: A non maskeable interrupt has been generated
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void nmi_interrupt_handler(){
    printf("Interrupt 2 - Nonmaskable Interrupt\n");
    system_handler(SYS_HALT,256,0,0);
}

/* breakpoint_handler
 *
 * DESCRIPTION: Breakpoint instruction was executed
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void breakpoint_handler(){
    printf("Interrupt 3 - Breakpoint Exception\n");
    system_handler(SYS_HALT,256,0,0);
}

/* overflow_handler
 *
 * DESCRIPTION: Overflow occured wtih arithmetic
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void overflow_handler(){
    printf("Interrupt 4 - Overflow Exception\n");
    system_handler(SYS_HALT,256,0,0);
}

/* bound_range_handler
 *
 * DESCRIPTION: Attempted to access index out of bounds
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void bound_range_handler(){
    printf("Interrupt 5 - BOUND Range Exceeded\n");
    system_handler(SYS_HALT,256,0,0);
}

/* invalid_opcode_handler
 *
 * DESCRIPTION: Processor attempted to execute an invalid
                instruction
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void invalid_opcode_handler(){
    printf("Interrupt 6 - Invalid Opcode\n");
    system_handler(SYS_HALT,256,0,0);
}

/* device_not_avail_handler
 *
 * DESCRIPTION: Processor executed instructions hardware
                unable to finish
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void device_not_avail_handler(){
    printf("Interrupt 7 - Device Not Available\n");
    system_handler(SYS_HALT,256,0,0);
}

/* dbl_fault_handler
 *
 * DESCRIPTION: Second exception generated while first exception
                being handled
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void dbl_fault_handler(){
    printf("Interrupt 8 - Double Fault\n");
    system_handler(SYS_HALT,256,0,0);
}

/* coprocess_seg_handler
 *
 * DESCRIPTION: Detected a page or segment violation
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void coprocess_seg_handler(){
    printf("Interrupt 9 - Coprocessor Segment Overrun\n");
    system_handler(SYS_HALT,256,0,0);
}

/* inval_tss_handler
 *
 * DESCRIPTION: Error occured with TSS
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void inval_tss_handler(){
    printf("Interrupt 10 - Invalid TSS\n");
    system_handler(SYS_HALT,256,0,0);
}

/* set_not_pres_handler
 *
 * DESCRIPTION: Attempts to access an invalid segment
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void seg_not_pres_handler(){
    printf("Interrupt 11 - Segment Not Present\n");
    system_handler(SYS_HALT,256,0,0);
}

/* stack_fault_handler
 *
 * DESCRIPTION: Limit violation regarded with the stack or
                not present stack segment
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void stack_fault_handler(){
    printf("Interrupt 12 - Stack Fault Exception\n");
    system_handler(SYS_HALT,256,0,0);
}

/* gen_protect_handler
 *
 * DESCRIPTION: One of many general protection violations
                RTDC
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void gen_protect_handler(){
    printf("Interrupt 13 - General Protection Exception\n");
    system_handler(SYS_HALT,256,0,0);
}

/*  page_fault_handler
 *
 * DESCRIPTION: Processor detected error regarding pages
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void page_fault_handler(){
    //clear();
    printf("Interrupt 14 - Page-Fault Exception\n");
    //while(1);
    system_handler(SYS_HALT,256,0,0);
}

/* float_point_handler
 *
 * DESCRIPTION: Floating point error occured
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void float_point_handler(){
    printf("Interrupt 16 - x87 FPU Floating-Point Error\n");
    system_handler(SYS_HALT,256,0,0);
}

/* align_check_handler
 *
 * DESCRIPTION: Detected unaligned memory when supposed to
                be aligned
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void align_check_handler(){
    printf("Interrupt 17 - Alignment Check Exception\n");
    system_handler(SYS_HALT,256,0,0);
}

/* machine_check_handler
 *
 * DESCRIPTION: Internal machine error or bus error
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void machine_check_handler(){
    printf("Interrupt 18 - Machine-Check Exception\n");
    system_handler(SYS_HALT,256,0,0);
}

/* simd_float_point_handler
 *
 * DESCRIPTION: SIMD floating point exception
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void simd_float_point_handler(){
    printf("Interrupt 19 - SIMD Floating-Point Exception\n");
    system_handler(SYS_HALT,256,0,0);
}
