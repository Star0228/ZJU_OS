#include "trap.h"


void trap_handler(uint64_t scause, uint64_t sepc, struct pt_regs *regs) {
    uint64_t scause_code = scause;
    uint64_t highest_bit = (scause_code >> 63) & 1;
    switch (highest_bit)
    {
    case 1:
        if(scause == 0x8000000000000005){
            // printk("[S] Supervisor Mode Timer Interrupt\n");
            clock_set_next_event();
            do_timer();
        }else{
            printk("Non-Timer Interrupt\n");
        }
        break;
    case 0:
        if(scause == 0x8){
            // printk("Environment call from U-mode\n");
            switch (regs->reg17)
            {
            case 64://sys_write
                sys_write(regs->reg10, (const char*)regs->reg11, (uint64_t)regs->reg12, regs);
                break;

            case 172://sys_getpid
                sys_getpid(regs);
                break;
            }
        }
        regs->sepc += 0x4;
    }

}