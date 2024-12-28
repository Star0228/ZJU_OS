#include "printk.h"
#include "stdint.h"

extern void test();
extern uint64_t* _stext;
extern uint64_t* _srodata;

int start_kernel() {
    printk("2024");
    printk(" ZJU Operating System\n");

    // printk("stext: %llx\n", &_stext);
    // printk("srodata: %llx\n", &_srodata);

    // if(*_stext = 0x1) {
    //     printk("stext write: pass!\n");
    // } 
    // if(*_srodata = 0x1) {
    //     printk("srodata write: pass!\n");
    // }

    // asm volatile("call _srodata");
    test();
    return 0;
}
