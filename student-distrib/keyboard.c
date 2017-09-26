// this is the implementation file for keyboard.h

#include "keyboard.h"
#include "lib.h"
#include "sys_handler_helper.h"


static uint8_t keyboard_input_make_array[47] = {

    0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21,     // A,B,C,D,E,F
    0x22, 0x23, 0x17, 0x24, 0x25, 0x26,     // G,H,I,J,K,L
    0x32, 0x31, 0x18, 0x19, 0x10, 0x13,     // M,N,O,P,Q,R
    0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D,     // S,T,U,V,W,X
    0x15, 0x2C, 0x0B, 0x02, 0x03, 0x04,     // Y,Z,0,1,2,3
    0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,      // 4,5,6,7,8,9
    0x29, 0x0C, 0x0D, 0x1A, 0x1B, 0x2B,     // `,-,=,[,],'\'
    0x27, 0x28, 0x33, 0x34, 0x35            // ;,'',',',.,/
};

//ascii values for the lower case numbers and symbols
static uint8_t ascii_val[47] = {

    'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9',
    '`', '-', '=', '[', ']', 0x5C,
    ';', 0x27, ',', '.', '/'

};

//ascii values for the upper case numbers and symbols
static uint8_t ascii_val_upper[47] = {

    'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9',
    '`', '-', '=', '[', ']', 0x5C,
    ';', 0x27, ',', '.', '/'
};

//ascii value table for shift pressed symbols and upper case letters
static uint8_t ascii_val_shift[47] = {

    'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', ')', '!', '@', '#',
    '$', '%', '^', '&', '*', '(',
    '~', '_', '+', '{', '}', '|',
    ':', '"', '<', '>', '?'

};

static uint8_t ascii_val_caps_shift[47] = {

    'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', ')', '!', '@', '#',
    '$', '%', '^', '&', '*', '(',
    '~', '_', '+', '{', '}', '|',
    ':', '"', '<', '>', '?'

};
//keyboard buffer
volatile uint8_t line_char_buffer[BUFFER_SIZE];

//flags for important keys
uint32_t capslock_flag;
uint32_t shift_flag;
uint32_t ctrl_flag;
uint32_t alt_flag;
uint32_t enter_flag;
uint32_t back_flag;
uint32_t first_flag;

//keeps track of the last active keyboard buffer index
int buffIdx;



/* keyboard_init
 *
 * DESCRIPTION: Enables keyboard IRQ on PIC
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: none
 */
void keyboard_init(void)
{
    // enable the IRQ on PIC associated with keyboard
    enable_irq(KEYBOARD_IRQ_NUM);

    //initialize the flags and the buffIdx
    buffIdx = -1;
    capslock_flag = 0;
    shift_flag = 0;
    ctrl_flag = 0;
    alt_flag = 0;
    back_flag = 0;
    first_flag = 1;

}

/* keyboard_handler
 *
 * DESCRIPTION: Handler called by IDT in response to a keyboard interrupt.
 *              Reads in make code of key from port, loops through an array to
 *              find the corresponding index for the appropriate Ascii value, and
 *              then puts the ascii value to screen.
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: Sends EOI to PIC(S)
 */
void keyboard_handler()
{
    /*
        1) create the mapping mechanism
        2) call end of interrupt signal
    */

    //wrap in critical section
    uint32_t f;
    cli_and_save(f);

    //end of interrupt signal
    send_eoi(KEYBOARD_IRQ_NUM);
    uint8_t keyboard_read;


    // take in the port value holding the make code for letter
    keyboard_read = inb(KEYBOARD_BUFFER_PORT);


    //choose what to do with the scan code imported from the keyboard port
    if (keyboard_read == SPACE_PRESS)
      space_press();
    else if (keyboard_read == ENTER_PRESS)
      enter_press();
    else if (keyboard_read == ENTER_RELEASE)
      enter_release();
    else if ((keyboard_read == CTRL_PRESS) || (keyboard_read == CTRL_RELEASE))
      CtrlStatus(keyboard_read);
    else if ((keyboard_read == ALT_PRESS) || (keyboard_read == ALT_RELEASE))
      AltStatus(keyboard_read);
    else if ((keyboard_read == L_CLEAR) && (ctrl_flag == 1))
      clearScreen();
    else if ((keyboard_read == F2_PRESS) && (alt_flag == 1) && (ctrl_flag == 0) && (shift_flag == 0)){
        restore_flags(f);
        switch_terminal(SHELL1);
    }
    else if ((keyboard_read == F1_PRESS) && (alt_flag == 1) && (ctrl_flag == 0) && (shift_flag == 0)){
        restore_flags(f);
        switch_terminal(SHELL0);
    }
    else if ((keyboard_read == F3_PRESS) && (alt_flag == 1) && (ctrl_flag == 0) && (shift_flag == 0)){
        restore_flags(f);
        switch_terminal(SHELL2);
    }
    else if ((keyboard_read == LSHIFT_PRESS) || (keyboard_read == LSHIFT_RELEASE) || (keyboard_read == RSHIFT_PRESS) || (keyboard_read == RSHIFT_RELEASE))
      LRshift(keyboard_read);
    else if (keyboard_read == CAPS)
      caps_on();
    else if (keyboard_read == BKSP)
      bksp_handler();
    else
      keyboardBuff(keyboard_read);

    restore_flags(f);
    return;
}

/* void keyboardBuff
 * inputs: keyboard scan code
 * outputs: none
 * side effects: adds to the keyboard buffer, outputs to screen
 * function: a key has been pressed so this function needs to add that key to the buffer
              and also output it onto the terminal screen
*/
void keyboardBuff(uint8_t keyboard_read) {

  //return if buffer overflow
  if(buffIdx == BUFFER_MAX_INDEX)
    return;

  int i;

  //add char to the buffer based on the scancode
  for(i = 0; i < 47; i++)
  {
    if(keyboard_read == keyboard_input_make_array[i])
      {
        //if (line_char_buffer[buffIdx] != ' ') {
            //increment buffer index if adding character
            buffIdx++;
          //check caps lock to choose correct ascii table to draw from
          if((capslock_flag == 1) && (shift_flag == 1)) {
            line_char_buffer[buffIdx] = ascii_val_caps_shift[i];
            putc(line_char_buffer[buffIdx]);
            break;
          }
          else if(capslock_flag == 1) {
            line_char_buffer[buffIdx] = ascii_val_upper[i];
            putc(line_char_buffer[buffIdx]);
            break;
          }
          //check for shift to choose correct ascii table to draw from
          else if (shift_flag == 1) {
            line_char_buffer[buffIdx] = ascii_val_shift[i];
            putc(line_char_buffer[buffIdx]);
            break;
          }
          else {
            //if neither shift or caps lock is selected, choose appropraite ascii table
            line_char_buffer[buffIdx] = ascii_val[i];
            putc(line_char_buffer[buffIdx]);
            break;
          }
      //  }

      }
  }

  return;

}

/* void space_press
 * inputs: none
 * outputs: none
 * side effects: prints space to screen
 * function: handler for when the space bar is clicked
 */
void space_press(){

    //check for buffer overflow and handle apporpriately
    if(buffIdx == BUFFER_MAX_INDEX  ){
      return;
    }
    buffIdx++;

    //print and add space to buffer and screen
    putc(' ');
    line_char_buffer[buffIdx] = ' ';
    return;
}

/* void enter_press
 * inputs: none
 * outputs: none
 * side effects: prints enter to screen, clears buffer
 * function: handler for when the enter key is clicked, clears buffer
 */
void enter_press(){
  enter_flag = 1;

  int32_t i;
  buffIdx = -1;
  if (line_char_buffer[0] == '\0') {
      for(i = 0; i < BUFFER_SIZE; i++){
          line_char_buffer[i] = '\0';
      }
      buffIdx++;
      line_char_buffer[buffIdx] = '\n';
      //print newline

        putc('\n');

      return;
  }
  else {
    for(i = 0; i < BUFFER_SIZE; i++){
        line_char_buffer[i] = '\0';
    }
    putc('\n');
    return;
  }
}

/* void enter_release
 * inputs: none
 * outputs: none
 * side effects: clears enter_flag
 * function: handler for when the enter key is released
 */
void enter_release(){
    enter_flag = 0;
}

/* void bksp_handler
 * inputs: none
 * outputs: none
 * side effects: deletes last character from screen, removes from buffer
 * function: handler for when the backspace is clicked
 */
void bksp_handler() {
  //check to see that buffer is not empty
  if(buffIdx == -1)
    return;
  line_char_buffer[buffIdx] = '\0';
  buffIdx--;
  //this function deletes last drawn char
  backspace();
  return;
}

/* void caps_on
 * inputs: none
 * outputs: none
 * side effects: sets caps flag
 * function: handler for when the caps lock is clicked
 */
void caps_on(){
  //change state of caps lock flag
  if (capslock_flag != 1)
    capslock_flag = 1;
  else
    capslock_flag = 0;
}

/* void LRshift
 * inputs: keyboard scan code
 * outputs: none
 * side effects: sets shift flag
 * function: handler for when the sshift bar is clicked
 */
void LRshift(uint8_t keyboard_read) {
  //set shift flag
  if ((keyboard_read == LSHIFT_PRESS) || (keyboard_read == RSHIFT_PRESS))
    shift_flag = 1;
  else
    shift_flag = 0;
}

/* void CtrlStatus
 * inputs: keyboard scancode
 * outputs: none
 * side effects: sets control button flag
 * function: handler for when the control bar is clicked
 */
void CtrlStatus(uint8_t keyboard_read) {
  //set control flag
  if (keyboard_read == CTRL_PRESS)
    ctrl_flag = 1;
  else
    ctrl_flag = 0;
}

/* void AltStatus
 * inputs: keyboard scancode
 * outputs: none
 * side effects: sets alt button flag
 * function: handler for when the alt bar is clicked
 */
void AltStatus(uint8_t keyboard_read) {
  //set control flag
  if (keyboard_read == ALT_PRESS)
    alt_flag = 1;
  else
    alt_flag = 0;
}


/* void clearScreen
 * inputs: none
 * outputs: none
 * side effects: clears the terminal screen, changes cursor location, clears buffer
 * function: clears the scren and sets the curssor to top left
 */
void clearScreen() {

  //clear sreen
  clear();

  //set cursor to top left
  resetCursor();

  clear_buffer();

  //pass empty
  line_char_buffer[0] = '\n';
  buffIdx = QUIT;
  return;
}

/* void get_buf_idx
 * inputs: none
 * outputs: buffIdx
 * side effects: none
 * function: getter function to get buffIdx
 */
int get_buf_idx(){
    return buffIdx;
}

/* void set_buf_idx
 * inputs: int32_t index
 * outputs: none
 * side effects: changes buffIdx
 * function: used to change buffer index
 */
void set_buf_idx(int32_t index){
  buffIdx = index;
}

/* keyboard_open
 * input: NONE
 * output: 0
 * side effects:
 * function:"opens" keyboard
 */
int32_t keyboard_open(){
    return 0;
};

/* keyboard_close
 * input: NONE
 * output: 0
 * side effects:
 * function:"closes" keyboard
 */
int32_t keyboard_close(){
    return 0;
}

/* keyboard_write
 * input: NONE
 * output: -1
 * side effects:
 * function:"writes" keyboard, not a real function
 */
int32_t keyboard_write(){
    return -1;
}

/* keyboard_read
 * input: the buffer to write to, bytes to write
 * output: the total number of bytes written
 * side effects: writes to a buffer whatever is in the keyboard buffer
 * function: takes the keyboard buffer and copies whatever is in it to a different
 *            buffer passed in by the function
 */
int32_t keyboard_read(char* buf, uint32_t byte_count){
    //dont let inside if buffer is empty
    while(buffIdx == -1);
    int i;
    //loop until user presses enter
  //  if(buffIdx != -2){
        do{
            for(i=0;i<byte_count;i++){
                buf[i] = line_char_buffer[i];
            }
            if (buffIdx == QUIT)
              enter_flag = 1;
        }while(enter_flag == 0);
  //  }


    //empty buffer is being passed
    if (line_char_buffer[0] == '\n') {
      buf[0] = '\0';
      line_char_buffer[0] = '\0';
      buffIdx = -1;
      enter_flag = 0;
      return 0;//i+1
    }

    // if the command is "exit", do not add new line character at the end of the string
    if (strncmp(buf,"exit",4) == 0) {
      first_flag = 1;
      return i + 1;
    }


    int j;
    j = 0;

    // loop through to find null terminating character
    while (buf[j] != '\0') {
      j++;
    }

    // fill null terminating space in buffer with nl char
     buf[j] = '\n';


    // return # of bytes
    return j+1;//i+2
}

/* keyboard_driver
 * input: uint32_t cmd - command number
 *        uint32_t fd - file descriptor
 *        void* buf - buffer to read/write
 *        uint32_t byte_count - how many bytes to read/write
 * output: return values from respective o/c/r/w functions, -1 if invalid cmd
 * side effects:
 * function: dispatcher function
 */
int32_t keyboard_driver(uint32_t cmd, uint32_t fd, void* buf, uint32_t byte_count){
    if(cmd == OPEN){
        return keyboard_open();
    }
    else if(cmd == READ){
        return keyboard_read((int8_t*)buf,byte_count);
    }
    else if(cmd == WRITE){
        return keyboard_write();
    }
    else if(cmd == CLOSE){
        return keyboard_close();
    }
    return -1;
}

void clear_buffer(){
  int32_t i;
  for(i = 0; i < BUFFER_SIZE; i++){
    line_char_buffer[i] = '\0';
  }
  buffIdx = -1;
}
