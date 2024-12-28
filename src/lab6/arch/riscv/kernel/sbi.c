#include "stdint.h"
#include "sbi.h"

struct sbiret sbi_ecall(uint64_t eid, uint64_t fid,
                        uint64_t arg0, uint64_t arg1, uint64_t arg2,
                        uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    struct sbiret ret;
    uint64_t error, value;
    asm volatile (
        "mv a7, %[ob_e]\n"
        "mv a6, %[ob_f]\n"
        "mv a0, %[ob_0]\n"
        "mv a1, %[ob_1]\n"
        "mv a2, %[ob_2]\n"
        "mv a3, %[ob_3]\n"
        "mv a4, %[ob_4]\n"
        "mv a5, %[ob_5]\n"
        "ecall\n"
        "mv %[error], a0\n"
        "mv %[value], a1\n"
        : [error] "=r" (error), [value] "=r" (value)
        : [ob_e] "r" (eid), [ob_f] "r" (fid),
            [ob_0] "r" (arg0), [ob_1] "r" (arg1), 
            [ob_2] "r" (arg2), [ob_3] "r" (arg3),
            [ob_4] "r" (arg4), [ob_5] "r" (arg5)
        : "memory","a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"
    );
    ret.error = error;
    ret.value = value;
    return ret;
}

struct sbiret sbi_debug_console_write_byte(uint8_t byte) {
    return sbi_ecall((uint64_t)0x4442434E,(uint64_t)2,byte,0,0,0,0,0);
}

struct sbiret sbi_system_reset(uint32_t reset_type, uint32_t reset_reason) {
    return sbi_ecall(0x53525354,0,reset_type,reset_reason,0,0,0,0);
}

struct sbiret sbi_set_timer(uint64_t stime_value){
    return sbi_ecall(0x54494D45,0,stime_value,0,0,0,0,0);
}

struct sbiret sbi_debug_console_write(unsigned long num_bytes,
 unsigned long base_addr_lo,
 unsigned long base_addr_hi){
    return sbi_ecall(0x4442434E,0,num_bytes,base_addr_lo,base_addr_hi,0,0,0);
 }


struct sbiret sbi_debug_console_read(unsigned long num_bytes,
    unsigned long base_addr_lo,unsigned long base_addr_hi){
    return sbi_ecall(0x4442434E,1,num_bytes,base_addr_lo,base_addr_hi,0,0,0);
}