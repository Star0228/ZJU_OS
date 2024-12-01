#include "../include/syscall.h"

extern struct task_struct *current;
extern struct task_struct *task[];
extern uint64_t nr_tasks;
extern uint64_t* swapper_pg_dir;
extern void __ret_from_fork();
extern void __dummy();


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

uint64_t do_fork(struct pt_regs *regs){
    printk("\n");
    struct task_struct *new_task = (struct task_struct*)alloc_page();
    task[nr_tasks] = new_task;
    memcpy((void*)new_task, (void*)current, PGSIZE);
    new_task->pid = nr_tasks++;//nr_tasks must be unused in the future 
    new_task->thread.ra = (uint64_t)&__ret_from_fork;

    new_task->thread.sp = (uint64_t)new_task + ((uint64_t)regs - PGROUNDDOWN((uint64_t)regs));
    new_task->thread.sscratch = csr_read(sscratch);//same as parent
                                                                                
    new_task->mm.mmap = NULL;

    struct pt_regs *forked_regs = (struct pt_regs *)(new_task->thread.sp);
    forked_regs->reg10 = 0;
    forked_regs->sepc += 0x4;
    forked_regs->reg2 = new_task->thread.sp;

    new_task->pgd = (uint64_t*)alloc_page();
    memcpy((void *)new_task->pgd, (void *)&swapper_pg_dir, PGSIZE);       

    for(struct vm_area_struct *vma = current->mm.mmap; vma ; vma = vma->vm_next){
        struct vm_area_struct *new_vma = (struct vm_area_struct*)alloc_page();
        memcpy((void*)new_vma, (void*)vma, sizeof(struct vm_area_struct));
        new_vma->vm_next = NULL;
        if(new_task->mm.mmap == NULL){
            new_task->mm.mmap = new_vma;                    
        }else{
            struct vm_area_struct *tmp = new_task->mm.mmap;
            while(tmp->vm_next){
                tmp = tmp->vm_next;
            }
            tmp->vm_next = new_vma;
            new_vma->vm_prev = tmp;
        }
        for(uint64_t page_addr = PGROUNDDOWN(vma->vm_start); page_addr < vma->vm_end; page_addr += PGSIZE){
            uint64_t vpn_2 = (page_addr >> 30) & 0x1ff;
            uint64_t vpn_1 = (page_addr >> 21) & 0x1ff;
            uint64_t vpn_0 = (page_addr >> 12) & 0x1ff;
            uint64_t pte_2 = current->pgd[vpn_2];
            if((pte_2 & 0x1) != 0x1){
                continue;
            }
            uint64_t pte_1 = ((uint64_t*)(((pte_2 & 0x003ffffffffffc00)<<2)+PA2VA_OFFSET))[vpn_1] ;
            if((pte_1 & 0x1) != 0x1){
                continue;
            }
            uint64_t pte_0 = ((uint64_t*)(((pte_1 & 0x003ffffffffffc00)<<2)+PA2VA_OFFSET))[vpn_0];
            if((pte_0 & 0x1) != 0x1){
                continue;
            }else{
                uint64_t new_phy_pg = (uint64_t)alloc_page();
                memcpy((void*)new_phy_pg, (void*)page_addr, PGSIZE);
                uint64_t perm = (pte_0 & 0xff) | 0x11;
                create_mapping(new_task->pgd, page_addr, (new_phy_pg-PA2VA_OFFSET), PGSIZE, perm);
            }
        }
    }
    printk("[PID = %d] forked from [PID = %d]\n\n", nr_tasks-1, current->pid);    
}