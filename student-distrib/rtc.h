#ifndef RTC_H
#define RTC_H

#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "keyboard.h"
#include "sys_handlers.h"
#include "idt.h"


#define RTC_PORT  0x70
#define RW_CMOS 0x71
#define STAT_REG_A  0x8A
#define STAT_REG_B  0x8B
#define STAT_REG_C  0x8C
#define ENABLE_BIT_SIX  0x40
#define RTC_IRQ_NUM 8
#define RATE 14
#define PORTANDER 0xF0
#define BYTE4 4
#define HZ2 2
#define HZ4 4
#define HZ8 8
#define HZ16 16
#define HZ32 32
#define HZ64 64
#define HZ128 128
#define HZ256 256
#define HZ512 512
#define HZ1024 1024
#define FREQ2 0x0F
#define FREQ4 0x0E
#define FREQ8 0x0D
#define FREQ16 0x0C
#define FREQ32 0x0B
#define FREQ64 0x0A
#define FREQ128 0x09
#define FREQ256 0x08
#define FREQ512 0x07
#define FREQ1024 0x06

extern void rtc_init(void);
extern void rtc_handler();

uint8_t int_flag;

int32_t open_rtc();
int32_t read_rtc();
int32_t write_rtc(const int32_t* buf);
int32_t close_rtc();


int32_t rtc_driver(uint32_t cmd, uint32_t fd, void* buf, uint32_t nbytes);
void set_freq(int32_t freq);


void test_rtc();

#endif
