//this is for code for scheduler
#include "schedule.h"

uint32_t sched_vid_backups[] = {BACKUP0,BACKUP1,BACKUP2};

/*pit_init
* input - none
* outpt - none
* side effects - enables pit interrpts on PIC
* description - initializes PIT for scheduler
*/
void pit_init(void)
{
    /*
        1) Set command byte to square wave
        2) Set low bytes of divisor
        3) Set high bytes of divisor
    */

    int32_t i;

    outb(PIT_MODE_3, MODE_COMMAND_REG);
    outb(DIV_100HZ & MASK_FREQ, PIT_0_DATA_PORT);
    outb(DIV_100HZ >> 8, PIT_0_DATA_PORT);

    // enable interrupts on PIC
    enable_irq(PIT_IRQ_NUM);
    for(i = 0; i < SCHED_SIZE; i++){
        schedule_arr[i] = NULL;
        new_process[i] = 1;
    }

    curr = 0;
}


/*pit_handler
* input - none
* outpt - none
* side effects - context switch to next scheduled process
* description - is used to implement round robin scheduling
*/
void pit_handler()
{
    send_eoi(PIT_IRQ_NUM);
    if(!setup)
        return;
 //   uint32_t flags;
  //  cli_and_save(flags);

    //move to next process
    int32_t last = curr;
    int32_t i;
    for(i = 0; i < SCHED_SIZE; i++){
        curr = (curr+1) % SCHED_SIZE;
        if(schedule_arr[curr])
            break;
    }

    /*if(new_process[curr]){
        new_process[curr] = 0;
        return;
    }*/
    //no processes to schedule or nothing to update
    if(last == curr || !schedule_arr[curr]){
        return;
    }
    /*
    if(schedule_arr[curr]->proc_id == 0 || schedule_arr[curr]->proc_id == 4
        || schedule_arr[curr]->proc_id == 8){
            if(schedule_arr[curr]->proc_id/4 != curr_terminal)
                return;
        }
        */



    //write code that turns on "display to screen" only for running process
    //processes running in background write to their own backups

    //if next process is not in the active terminal write to backup video memory
    if((schedule_arr[curr]->proc_id)/4 != curr_terminal){
        int term_to_write = (curr_pcb->proc_id)/4;
        page_directory[VIDMAP_PAGE] = (uint32_t)page_table_vid | URWON;
        page_table_vid[0] = sched_vid_backups[term_to_write] | URWON;
        page_table[VIDEO >> 12] = sched_vid_backups[term_to_write] | RWON;
    }
    //next process is in current terminal so write to active memory
    else{
      page_directory[VIDMAP_PAGE] = (uint32_t)page_table_vid | URWON;
      page_table_vid[0] = VIDEO | URWON;
      page_table[VIDEO >> 12] = VIDEO | URWON;
    }

    //flush tlb
   asm volatile(
       "movl %cr3,%eax \n \
       movl %eax,%cr3"
   );


    //context switching
    tss.esp0 = (uint32_t)(schedule_arr[curr])+STACK_SIZE4;
    tss.ss0 = KERNEL_DS;

    //save old esp/ebp
    if(schedule_arr[last]){
        asm (
          "movl %%esp, %0"
          :"=r"(schedule_arr[last]->sched_esp)
        );
        asm (
          "movl %%ebp, %0"
          :"=r"(schedule_arr[last]->sched_ebp)
        );
    }

    //restore new esp/ebp
    asm (
      "movl %0, %%esp"
      :
      :"r"(schedule_arr[curr]->sched_esp)
    );
    asm (
      "movl %0, %%ebp"
      :
      :"r"(schedule_arr[curr]->sched_ebp)
    );

    //set paging to new process
    page_directory[32] = mem_locs[schedule_arr[curr]->idx] | SURWON;

    //flush tlb
    asm volatile(
        "movl %cr3,%eax \n \
        movl %eax,%cr3"
    );
    curr_pcb = schedule_arr[curr];

   // restore_flags(flags);
}
