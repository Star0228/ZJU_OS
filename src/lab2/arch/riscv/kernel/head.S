    .extern start_kernel 
    .extern _traps
    .extern sbi_set_timer
    .extern task_init
    .extern mm_init
    .section .text.init
    .globl _start 
_start:
    la sp, boot_stack_top

    la t0, _traps
    csrw stvec, t0
    li t0, 0x20           
    csrs sie, t0           
    rdtime t0
    li t1, 10000000
    add a0, t0, t1
    call sbi_set_timer
    li t0, 0x2
    csrs sstatus, t0

    call mm_init
    call task_init
    call start_kernel

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096
    .globl boot_stack_top
boot_stack_top: