#include "../include/syscall.h"

extern struct task_struct *current;

void sys_write(unsigned int fd, const char* buf, uint64_t count, struct pt_regs *regs){
    
    if(fd == 1){
        uint64_t i = 0;
        for( ; i < count; i++){
            printk("%c", buf[i]);
        }
        regs->reg10 = i;
    }
}


void sys_getpid(struct pt_regs *regs){
    regs->reg10 = current->pid;
}