#include "stdint.h"
#include "printk.h"
#include "clock.h"
#include "syscall.h"



void trap_handler(uint64_t scause, uint64_t sepc, struct pt_regs *regs);