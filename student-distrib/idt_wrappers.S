
.globl keyboard_handler_wrapper
.globl pit_handler_wrapper
.globl rtc_handler_wrapper
.globl system_handler_wrapper
.globl exec_ret


.data
    SKIP = 4
# keyboard_handler_wrapper
# Description: wrapper for keyboard interrupt handler to follow proper
#               interrupt stack convention
# INPUT/OUTPUT: none
# SIDE EFFECTS: none
keyboard_handler_wrapper:
  pusha
  call keyboard_handler
  popa
  iret

# pit_handler_wrapper
# Description: wrapper for PIT interrupt handler to follow proper
#               interrupt stack convention
# INPUT/OUTPUT: none
# SIDE EFFECTS: none
pit_handler_wrapper:
  pusha
  call pit_handler
  popa
  iret

# rtc_handler_wrapper
# Description: wrapper for rtc interrupt handler to follow proper
#               interrupt stack convention
# INPUT/OUTPUT: none
# SIDE EFFECTS: none
rtc_handler_wrapper:
  pusha
  call rtc_handler
  popa
  iret

# system_handler_wrapper
# Description: wrapper for system interrupt handler to follow proper
#               interrupt stack convention
# INPUT/OUTPUT: none
# SIDE EFFECTS: none
system_handler_wrapper:

  pushl %ebp
  pushl %esi
  pushl %edi
  pushl %edx
  pushl %ecx
  pushl %ebx
  pushl %eax
  call system_handler
system_return:
  addl $SKIP, %esp
  popl %ebx
  popl %ecx
  popl %edx
  popl %edi
  popl %esi
  popl %ebp

  # preserve eax as return value
  iret

exec_ret:
    movl -4(%ebp),%eax
    leave
    ret
