    .extern start_kernel 
    .extern _traps
    .extern sbi_set_timer
    .extern task_init
    .extern mm_init

    .extern setup_vm
    .extern early_pgtbl
    .extern setup_vm_final

    .extern _srodata

    .section .text.init
    .globl _start 

_start:
    la sp, boot_stack_top

    call setup_vm
    call relocate

    call mm_init
    call setup_vm_final
    call task_init

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

    call start_kernel

relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before) 

    li t0,0xffffffdf80000000
    add ra, ra, t0
    add sp, sp, t0

    # need a fence to ensure the new translations are in use
    sfence.vma zero, zero

    # set satp with early_pgtbl
    la t1, early_pgtbl
    srli t1, t1, 12
    li t0, 0x8
    slli t0, t0, 30
    slli t0, t0, 30
    or t0, t0, t1
    csrw satp, t0
    ret

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096
    .globl boot_stack_top
boot_stack_top: