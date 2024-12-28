#include "trap.h"

extern struct task_struct *current; //当前运行的task
extern char* _sramdisk;

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
            Err("[S] Unhandled Interrupt: scause=%d, sepc=%llx", scause, sepc);
        }
        break;
    case 0:
        if(scause == 0x8){
            // printk("Environment call from U-mode\n");
            switch (regs->reg17)
            {
            case SYS_WRITE://sys_write
                sys_write(regs->reg10, (const char*)regs->reg11, (uint64_t)regs->reg12, regs);
                break;
            case SYS_GETPID://sys_getpid
                // Log("[PID = %d PC = 0x%lx] sys_getpid",current->pid,regs->sepc);
                sys_getpid(regs);
                break;
            case SYS_CLONE://sys_clone
                // Log("[PID = %d PC = 0x%lx] sys_clone",current->pid,regs->sepc);
                regs->reg10 = do_fork(regs);
                break;
            }
            regs->sepc += 0x4;
        }else if(scause == 12||scause == 13||scause == 15){
            Log("[PID = %d PC = 0x%lx] valid page fault at `0x%llx` with cause %d",current->pid,regs->sepc,regs->stval,scause);
            do_page_fault(regs);
        }else{
            // regs->sepc += 0x4;
            // Err("[S] Unhandled Exception: scause=%d, sepc=%llx", scause, sepc);
        }
        
    }

}

void do_page_fault(struct pt_regs *regs) {
    struct vm_area_struct *vma = find_vma(&current->mm, regs->stval);
    if(vma == NULL){
        Err("Page Fault at [0x%lx]\n", regs->stval);
    }else{
        //如果非法（比如触发的是 instruction page fault 但 vma 权限不允许执行），则 Err 输出错误信息
        // if(((vma->vm_flags & VM_EXEC) == 0) ){
        //     Err("Instruction page Fault at [0x%lx] with no exec permission\n", regs->stval);
        // }
        //如果当前访问的虚拟地址在 VMA 中存在记录，则需要判断产生异常的原因：
            //如果是匿名区域，那么开辟一页内存，然后把这一页映射到产生异常的 task 的页表中
            //如果不是，则访问的页是存在数据的（如代码），需要从相应位置读取出内容，然后映射到页表中
        uint64_t new_pg = (uint64_t)alloc_page();
        memset((void*)new_pg, 0, PGSIZE);
        uint64_t offset = vma->vm_start & (PGSIZE-0x1);
        uint64_t elf_beginning = (uint64_t)(&_sramdisk) + offset;
        uint64_t end_offset_rev = PGSIZE - (vma->vm_start + vma->vm_filesz) & (PGSIZE-0x1);
        uint64_t perm = ((vma->vm_flags & VM_EXEC) ? 0x8 :0x0) | ((vma->vm_flags & VM_WRITE) ? 0x4 :0x0) |((vma->vm_flags & VM_READ) ? 0x2 :0x0);//|XWR
        perm |= 0x11; //U│V
        bool is_in_fstpg = PGROUNDDOWN(regs->stval) == PGROUNDDOWN((uint64_t)vma->vm_start);
        bool is_in_lstpg = PGROUNDDOWN(regs->stval) == PGROUNDDOWN((uint64_t)(vma->vm_start + vma->vm_filesz -1));
        if(!(vma->vm_flags & VM_ANON)){
            if(is_in_fstpg){
                if(is_in_lstpg){
                    memcpy((void*)(new_pg+offset), (void*)((uint64_t)(&_sramdisk)+offset), PGSIZE-offset-end_offset_rev);
                }else{
                    memcpy((void*)(new_pg+offset), (void*)((uint64_t)(&_sramdisk)+offset), PGSIZE-offset);
                }
            }else{
                if(is_in_lstpg){
                    memcpy((void*)new_pg, (void*)(elf_beginning+(PGROUNDDOWN(regs->stval) - vma->vm_start)), PGSIZE - end_offset_rev);
                }else{
                    memcpy((void*)new_pg, (void*)(elf_beginning+(PGROUNDDOWN(regs->stval) - vma->vm_start)), PGSIZE);
                }
            }
        }
        create_mapping(current->pgd, PGROUNDDOWN(regs->stval), (uint64_t)new_pg - PA2VA_OFFSET, PGSIZE, perm);
    }
}