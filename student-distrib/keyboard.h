// this is the header file for initializing the keyboard
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "sys_handlers.h"


#define KEYBOARD_BUFFER_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_IRQ_NUM 1
#define CAPS 0x3A
#define BKSP 0x0E
#define SCROLL_UP 0x48
#define SCROLL_DOWN 0x50
#define LSHIFT_PRESS 0x2A
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_PRESS 0x36
#define RSHIFT_RELEASE 0xB6
#define CTRL_PRESS 0x1D
#define CTRL_RELEASE 0x9D
#define ALT_PRESS 0x38
#define ALT_RELEASE 0xB8
#define F1_PRESS 0x3B
#define F2_PRESS 0x3C
#define F3_PRESS 0x3D
#define L_CLEAR   0x26
#define ENTER_PRESS 0x1C
#define ENTER_RELEASE 0x9C
#define SPACE_PRESS 0x39
#define BUFFER_MAX_INDEX 127
#define BUFFER_SIZE 129
#define QUIT -2


extern void keyboard_init(void);
extern void keyboard_handler();
extern void caps_on();
extern void enter_press();
extern void enter_release();
extern void space_press();
extern void bksp_handler();
extern void scrollPage();
extern void keyboardBuff(uint8_t keyboard_read);
extern void output_buffer();
extern void LRshift(uint8_t keyboard_read);
extern void CtrlStatus(uint8_t keyboard_read);
extern void AltStatus(uint8_t keyboard_read);
extern void clearScreen();
extern int get_buf_idx();
extern void set_buf_idx(int32_t index);
extern void clear_buffer();

int32_t keyboard_open();
int32_t keyboard_close();
int32_t keyboard_read(int8_t* buf, uint32_t byte_count);
int32_t keyboard_driver(uint32_t cmd, uint32_t fd, void* buf, uint32_t byte_count);

volatile extern uint8_t line_char_buffer[BUFFER_SIZE];


#endif
