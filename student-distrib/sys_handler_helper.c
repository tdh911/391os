#include "sys_handler_helper.h"


//Arrays of backups for different terminals
uint32_t vid_backups[] = {BACKUP0,BACKUP1,BACKUP2};
uint8_t buf_backup0[BUFFER_MAX_INDEX+1];
uint8_t buf_backup1[BUFFER_MAX_INDEX+1];
uint8_t buf_backup2[BUFFER_MAX_INDEX+1];
uint8_t *buf_backups[] = {buf_backup0,buf_backup1,buf_backup2};
int32_t buff_idx_backups[NUM_TERMINALS];
uint32_t xcoord_backups[NUM_TERMINALS];
uint32_t ycoord_backups[NUM_TERMINALS];
process_control_block_t *pcb_backups[NUM_TERMINALS];


/* init_shell
* input: none
* output: none
* side effects: places programs into memory, sets up paging
* description: intializes a shell in memory without calling execute, basically
                a fake execute that only loads into memory
*/
void init_shell(){
    //error check
    if(num_processes >= 2)
        return;

    dentry_t d;
    int32_t i;
    int32_t length;
    int32_t proc_idx;
    //increment processes
    num_processes++;

    //find space for current process on kernel stack
    task_stack_t *process;
    for(i = 1; i < MAX_PROCESS; i++){
        //find first available kernel stack
        if(tasks->task[i].in_use == OFF){
           tasks->task[i].in_use = ON;
           process = &tasks->task[i];
           proc_idx = i;
           break;
        }
    }
    dread("shell",&d);

    //set up paging
    page_directory[USER_PROG] = mem_locs[proc_idx] | SURWON;
    process->proc.idx = proc_idx;

    //flush tlb
    asm volatile(
        "movl %cr3, %eax \n \
        movl %eax, %cr3");


    /*-------------------
    LOAD FILE INTO MEMORY
    ---------------------*/
    length = get_length(d.inode_num);
    int8_t *prog_ptr = (int8_t*)USER_ENTRY;
    fread(d.inode_num,0,prog_ptr,length);

    //set up pcb id
    if(proc_idx == 1)
        process->proc.proc_id = 4;
    if(proc_idx == 2)
        process->proc.proc_id = 8;

    //open stdin
    process->proc.file_arr[0].flags = ON;
    process->proc.file_arr[0].table = keyboard_driver;

    //open stdout
    process->proc.file_arr[1].flags = ON;
    process->proc.file_arr[1].table = terminal_driver;

    //initialize "in use" flags to 0
    for(i=2; i<MAX_FD; i++)
        process->proc.file_arr[i].flags = OFF;
}

/* init_kernel_memory
* input: none
* output: none
* side effects: none
* description: initializes both other terminals in kernel memory
                to be able to be used with multiple terminal functionality.
*/

void init_kernel_memory(){
    int32_t i;

    //set pointer to tasks structure
    tasks = (kernel_tasks_t*)(KERNEL_BOT - MAX_PROCESS * STACK_SIZE);

    //initialize task to off, prevents page faulting
    for(i = 0; i < MAX_PROCESS; i++){
        tasks->task[i].in_use = OFF;
    }

    tasks->task[0].proc.proc_id = 0;

    //initialize me_locs array to point to correct physical pages for processes
    mem_locs[0] = TERMINAL0;
    mem_locs[1] = TERMINAL1;
    mem_locs[2] = TERMINAL2;
    mem_locs[3] = PROCESS0;
    mem_locs[4] = PROCESS1;
    mem_locs[5] = PROCESS2;

    num_processes = 0;

    //set first term
    curr_terminal = SHELL0;

    //shell dirt flag means 1 for visited
    shell_dirty = 0x001;

    //initialize other terminals
    init_shell();
    init_shell();
    setup = 0;
}

/* switch_terminal
* input: shell to switch to
* output: none
* side effects: changes esp and ebp, changes backups in memory
* description: the main function to switch a terminal.
                  1) saves current terminal by backing up keyboard, video mem, coords
                  2) saves esp and ebp for context switch
                  3) opens up new terminal by loading latest backups from keybaord, video mem, coords
                  4) loads in new esp and ebp for context switch
                  5) changes paging
*/

void switch_terminal(int32_t shell){

    //error check
    if(shell == curr_terminal || shell < SHELL0 || shell > SHELL2)
      return;

    //save video memory and cursor position of current task and keyboard
    xcoord_backups[curr_terminal] = coordReturn(1);
    ycoord_backups[curr_terminal] = coordReturn(0);
    buff_idx_backups[curr_terminal] = get_buf_idx();
    memcpy((void*)vid_backups[curr_terminal],(const void*)VIDEO,PG_SIZE);
    memcpy((void*)buf_backups[curr_terminal],(const void*)line_char_buffer,BUFFER_MAX_INDEX+1);

    //clear screen
    clear();

    //save pcb into backups
    pcb_backups[curr_terminal] = curr_pcb;

    //save esp and ebp into pcb
    asm (
        "movl %%ebp, %0"
        :"=r"(curr_pcb->sched_ebp)
      );
    asm (
        "movl %%esp, %0"
        :"=r"(curr_pcb->sched_esp)
      );

    //update curr terminal
    curr_terminal = shell;

    //creating terminal for the first time
    if(((0x1 << curr_terminal) & shell_dirty) == 0){
      /*
        asm (
            "movl %%ebp, %0"
            :"=r"(curr_pcb->sched_ebp)
          );
        asm (
            "movl %%esp, %0"
            :"=r"(curr_pcb->sched_esp)
          );
          */
 //       schedule_arr[curr_terminal] = &(tasks->task[curr_terminal].proc);

        //update curr pcb
        curr_pcb = &(tasks->task[curr_terminal].proc);
        clear_buffer();
        resetCursor();
        shell_dirty |= 0x1 << curr_terminal;
        //allows for keyboard interrupts
        sti();
        //call execute for second shell and third shell
        system_handler(SYS_EXECUTE,(uint32_t)"shell123",0,0);
    }

    //ELSE load video memory and cursor location and keyboard
    else{
      memcpy((void*)VIDEO,(const void*)vid_backups[curr_terminal],PG_SIZE);
      memcpy((void*)line_char_buffer,(const void*)buf_backups[curr_terminal],BUFFER_MAX_INDEX+1);
      set_buf_idx(buff_idx_backups[curr_terminal]);
      placeCursor(xcoord_backups[curr_terminal],ycoord_backups[curr_terminal]);
//      /*

      //update curr pcb
      curr_pcb = pcb_backups[curr_terminal];

      //save tss vals
      tss.esp0 = (uint32_t)(curr_pcb)+STACK_SIZE4;
      tss.ss0 = KERNEL_DS;

      //load new esp and ebp for context switch
      asm (
          "movl %0, %%ebp"
          :
          :"r"(curr_pcb->sched_ebp)
        );
      asm (
            "movl %0, %%esp"
            :
            :"r"(curr_pcb->sched_esp)
        );

        //repage
        page_directory[32] = mem_locs[curr_pcb->idx] | SURWON;
        //flush tlb
        asm volatile(
            "movl %cr3,%eax \n \
            movl %eax,%cr3"
        );

    }
}

/* demote_pcb_backup
* input: shell, pcb to demote
* output: none
* side effects: none
* description: takes a pcb and creates the outermost process
*/

void demote_pcb_backup(int32_t shell, process_control_block_t* pcb){
    pcb_backups[shell] = pcb;
}
