#ifndef IDT_WRAP
#define IDT_WRAP

extern void keyboard_handler_wrapper();

extern void rtc_handler_wrapper();

extern void system_handler_wrapper();

extern void pit_handler_wrapper();
#endif
