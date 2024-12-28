#include "stdint.h"
#include "printk.h"
#include "clock.h"
#include "syscall.h"
#include "proc.h"

#define SYS_WRITE   64

#define SYS_GETPID  172

#define SYS_CLONE 220

void trap_handler(uint64_t scause, uint64_t sepc, struct pt_regs *regs);

void do_page_fault(struct pt_regs *regs);