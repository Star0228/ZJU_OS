// #include "sbi.h"

// void test() {
//     sbi_system_reset(SBI_SRST_RESET_TYPE_SHUTDOWN, SBI_SRST_RESET_REASON_NONE);
//     __builtin_unreachable();
// }
#include "printk.h"
#include "defs.h"
void test() {
    int i = 0;
    while (1) {
        if ((++i) % 100000000 == 0) {
            printk("kernel is running!\n");
            i = 0;
            
            // uint64_t read = csr_read(sstatus);
            // printk("sstatus: %lx\n", read);

            // uint64_t read = csr_read(sscratch);
            // printk("old sscratch: %lx\n", read);
            // csr_write(sscratch, 0x20241005);
            // read = csr_read(sscratch);
            // printk("new sscratch: %lx\n", read);
        }
    }
}