#include "rtc.h"

uint8_t cur_val;
uint8_t disp_handler;

/* rtc_init
 *
 * DESCRIPTION: Initializes RTC and enables its respective IRQ on the PIC
 *              Enables bit 6 in status register b to turn on periodic interrupts
 *
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: Rewrites few bits in status registers A and B to enable periodic interrupts and set rate
 */
void rtc_init(void) {
  uint32_t flags;
  cli_and_save(flags);

  outb(STAT_REG_B, RTC_PORT);             // Select status register B and disable NMI
  cur_val = inb(RW_CMOS);                 // load cur_val with value in status register B
  outb(STAT_REG_B, RTC_PORT);             // reset the index back into RTC_PORT
  outb(cur_val | ENABLE_BIT_SIX, RW_CMOS);  // "OR" current value with 0x40 and write into CMOS port

  /* need to figure out how to change rate, am working on that */
  outb(STAT_REG_A, RTC_PORT);             // Select status register A and disable NMI
  cur_val = inb(RW_CMOS);                 // load cur_val with value in status register A
  outb(STAT_REG_A, RTC_PORT);             // reset the index back into RTC_PORT
  outb((cur_val & PORTANDER) | RATE, RW_CMOS);

  int_flag = 0;
  disp_handler = 0;

  enable_irq(RTC_IRQ_NUM);                // enable on PIC

  restore_flags(flags);
}

/* rtc_handler
 *
 * DESCRIPTION: Calls on test_interrupts function given for checkpoint 1. This function
 *              is called by IDT and handles RTC interrupts
 *
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: Clears content in register C to enable new interrupts. Send EOI to PIC
 */
void rtc_handler() {

  //test_interrupts();
  outb(STAT_REG_C,RTC_PORT);
  inb(RW_CMOS);

  if(disp_handler) putc('1');
  if(int_flag == 0)
    int_flag = 1;
  else
    int_flag = 0;
  send_eoi(RTC_IRQ_NUM);

}

/* open_rtc
 *
 * DESCRIPTION: Opens RTC driver
 *
 * INPUT/OUTPUT: input - filename
                 output - return 0
 * SIDE EFFECTS: Sets frequency to 2Hz
 */
int32_t open_rtc()
{
    // Set frequency to 2Hz
    set_freq(HZ2);

    return 0;
}

/* read_rtc
 *
 * DESCRIPTION: Waits until interrupt is finished before resetting flag
 *
 * INPUT/OUTPUT: inputs - file descriptor
                          buffer
                          number of bytes to be read
                 outputs - return 0
 * SIDE EFFECTS: Sets interrupt flag
 */
int32_t read_rtc()
{
    //Create local flag variable to check for changes in global
    uint8_t local_flag = int_flag;

    // Wait for interrupts to be finished
    while(local_flag == int_flag)
    {
        //wait for interrupt handler to finish
    }

    return 0;
}

/* write_rtc
 *
 * DESCRIPTION: Sets the frequency of the RTC
 *
 * INPUT/OUTPUT: inputs - file descriptor
                          buffer
                          number of bytes to be read
                 outputs - return -1 if write can't be performed
                           return number of bytes if write is successful
 * SIDE EFFECTS: Sets frequency
 */
int32_t write_rtc(const int32_t* buf)
{
    int32_t write_freq;

    // If number of bytes is not 4 or buffer is null, return -1
    if(buf == NULL)
        return -1;

    write_freq = *(buf);

    // Set the frequency
    set_freq(write_freq);

    return 0;
}

/* close_rtc
 *
 * DESCRIPTION: RTC frequency gets reset
 *
 * INPUT/OUTPUT: input - file descriptor
                 output - return 0
 * SIDE EFFECTS: None
 */
int32_t close_rtc()
{
    // Set frequency to 2Hz
    set_freq(HZ2);

    return 0;
}

/* rtc_driver
 * input: uint32_t cmd - command number
 *        uint32_t fd - file descriptor
 *        void* buf - buffer to read/write
 *        uint32_t byte_count - how many bytes to read/write
 * output: return values from respective o/c/r/w functions, -1 if invalid cmd
 * side effects:
 * function: dispatcher function
 */
int32_t rtc_driver(uint32_t cmd, uint32_t fd, void* buf, uint32_t nbytes){
    if(cmd == OPEN){
        return open_rtc();
    }
    else if(cmd == READ){
        return read_rtc();
    }
    else if(cmd == WRITE){
        return write_rtc((const int32_t*)buf);
    }
    else if(cmd == CLOSE){
        return close_rtc();
    }
    return -1;
}


/* set_freq
 *
 * DESCRIPTION: Changes the frequency of RTC
 *
 * INPUT/OUTPUT: input - new frequency
 * SIDE EFFECTS: Sets frequency, however is clipped at 1024 Hz
 */
void set_freq(int32_t freq)
{
    uint8_t new_freq, old_freq;

    // Save old frequency value
    outb(STAT_REG_A, RTC_PORT);
    old_freq = inb(RW_CMOS);

    // Using the new rate, determine which register A code needs to be used
    switch(freq)
    {
        case HZ2:
            new_freq = FREQ2;
            break;

        case HZ4:
            new_freq = FREQ4;
            break;

        case HZ8:
            new_freq = FREQ8;
            break;

        case HZ16:
            new_freq = FREQ16;
            break;

        case HZ32:
            new_freq = FREQ32;
            break;

        case HZ64:
            new_freq = FREQ64;
            break;

        case HZ128:
            new_freq = FREQ128;
            break;

        case HZ256:
            new_freq = FREQ256;
            break;

        case HZ512:
            new_freq = FREQ512;
            break;

        case HZ1024:
            new_freq = FREQ1024;
            break;

        default:
            return;
    }

    // Write the new frequency to the appropriate port
    outb(STAT_REG_A, RTC_PORT);
    outb((old_freq & PORTANDER) | new_freq, RW_CMOS);
}

/* test_rtc
 *
 * DESCRIPTION: Tests the RTC for full functionality
 *
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: Test RTC
 */
void test_rtc(){
    int i,key;
    int freq[10] = {2,4,8,16,32,64,128,256,512,1024}; // Test each frequency value
    disp_handler = 1;
    clear();
    resetCursor();
    // Test each frequency using enter button as progression to next frequency
    for(i = 0; i < 10; i++){
        key = get_buf_idx();
        write_rtc(&freq[i]);
        while(key == get_buf_idx());
        bksp_handler();
    }
    disp_handler = 0;
}
