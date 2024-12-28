#include "stdint.h"
#include "printk.h"
#include "clock.h"
#include "proc.h"

void trap_handler(uint64_t scause, uint64_t sepc) {
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
    default:
        printk("Exception\n");
        break;
    }

}