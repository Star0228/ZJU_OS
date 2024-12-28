#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "stdint.h"
#include "sbi.h"
#include "stddef.h"
#include "printk.h"
#include "proc.h"
#include "fs.h"


#define SYS_OPENAT  56
#define SYS_CLOSE   57
#define SYS_LSEEK   62
#define SYS_READ    63
#define SYS_WRITE   64
#define SYS_GETPID  172
#define SYS_CLONE   220

//     High Addr ───►  ┌─────────────┐
//                     │     x0      │
//                     │             │
//                     │     x1      │
//                     │             │
//                     │     x31     │
//                     │             │
//                     │      .      │
//                     │      .      │
//                     │      .      │
//                     │             │
//                     │     sepc    │
//                     │             │
//                     │  sstatus    │
//                     │    stval    │
//  sp (pt_regs)  ──►  ├─────────────┤
//                     │             │
//                     │             │
//                     │             │
//                     │             │
//                     │             │
//                     │             │
//                     │             │
//                     │             │
//                     │             │
//     Low  Addr ───►  └─────────────┘

struct pt_regs
{
    //因为我的保存上下文顺序不一样，从高地址开始存储x0-x31
    //所以这里的顺序也是从x31-x0开始捕获
    uint64_t stval;//added in Lab5
    uint64_t sstatus;
    uint64_t sepc;
    uint64_t reg31;
    uint64_t reg30;
    uint64_t reg29;
    uint64_t reg28;
    uint64_t reg27;
    uint64_t reg26;
    uint64_t reg25;
    uint64_t reg24;
    uint64_t reg23;
    uint64_t reg22;
    uint64_t reg21;
    uint64_t reg20;
    uint64_t reg19;
    uint64_t reg18;
    uint64_t reg17;
    uint64_t reg16;
    uint64_t reg15;
    uint64_t reg14;
    uint64_t reg13;
    uint64_t reg12;
    uint64_t reg11;
    uint64_t reg10;
    uint64_t reg9;
    uint64_t reg8;
    uint64_t reg7;
    uint64_t reg6;
    uint64_t reg5;
    uint64_t reg4;
    uint64_t reg3;
    uint64_t reg2;
    uint64_t reg1;
    uint64_t reg0;    
};

void sys_write(unsigned int fd, const char* buf, uint64_t count, struct pt_regs *regs);

int64_t my_sys_write(uint64_t fd, const char *buf, uint64_t len);

void do_syscall(struct pt_regs* regs);

void sys_getpid(struct pt_regs *regs);

uint64_t do_fork(struct pt_regs *regs);

#endif