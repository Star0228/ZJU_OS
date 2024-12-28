#include "mm.h"
#include "defs.h"
#include "proc.h"
#include "stdlib.h"
#include "printk.h"
#include "string.h"
#include "vm.h"
#include "elf.h"

extern void __dummy();
extern void __switch_to(struct task_struct *prev, struct task_struct *next);

//lab4 add
extern uint64_t* swapper_pg_dir;
extern char* _sramdisk;
extern char* _eramdisk;

void load_program(struct task_struct *task) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)&_sramdisk;
    Elf64_Phdr *phdrs = (Elf64_Phdr *)((char*)(&_sramdisk) + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        Elf64_Phdr *phdr = phdrs + i;
        if (phdr->p_type == PT_LOAD) {
            // alloc space and copy content
            uint64_t offset = phdr->p_vaddr & (PGSIZE-0x1);
            uint64_t* elf_copyed = (uint64_t*)alloc_pages((((uint64_t)phdr->p_memsz+(uint64_t)offset) / PGSIZE) + 1);
            memcpy((void*)(elf_copyed+offset), (void*)((char*)&_sramdisk+phdr->p_offset), (uint64_t)phdr->p_filesz);
            memset((void*)(elf_copyed+offset+phdr->p_filesz), 0, (uint64_t)(phdr->p_memsz - phdr->p_filesz));
            
            // do mapping
            uint64_t perm = ((phdr->p_flags & PF_X) ? 0x8 :0x0) | ((phdr->p_flags & PF_W) ? 0x4 :0x0) |((phdr->p_flags & PF_R) ? 0x2 :0x0);//|XWR
            perm |= 0x11; //U│V
            create_mapping(task->pgd, phdr->p_vaddr, (uint64_t)elf_copyed - PA2VA_OFFSET, (uint64_t)phdr->p_memsz+(uint64_t)offset, perm);//U│X│W│R│V
        }
    }
    task->thread.sepc = ehdr->e_entry;
}
//end lab4 add


struct task_struct *idle;           // idle process
struct task_struct *current;        // 指向当前运行线程的 task_struct
struct task_struct *task[NR_TASKS]; // 线程数组，所有的线程都保存在此



void task_init() {
    srand(2024);

    idle = (struct task_struct*)kalloc();
    idle->state = TASK_RUNNING;
    idle->counter = idle->priority = idle->pid = 0;
    current = task[0] = idle;

    for(int i = 1; i < NR_TASKS; i++) {
        task[i] = (struct task_struct*)kalloc();
        task[i]->state = TASK_RUNNING;
        task[i]->pid = i;
        task[i]->counter = 0;
        task[i]->priority = PRIORITY_MIN + rand() % (PRIORITY_MAX - PRIORITY_MIN + 1);
        task[i]->thread.ra = (uint64_t)&__dummy;
        task[i]->thread.sp = (uint64_t)task[i] + PGSIZE;
        //added in Lab4---------------------------------------------
        task[i]->thread.sepc = USER_START;
        task[i]->thread.sstatus |=  (uint64_t)0x40020;  //SPIE(5): 1, SUM(18): 1
        task[i]->thread.sstatus &= ~((uint64_t)0x100);  //SPP: 0
        task[i]->thread.sscratch = USER_END;

        //reuse swapper_pg_dir in user space
        task[i]->pgd = (uint64_t*)alloc_page();
        memcpy((void *)task[i]->pgd, (void *)&swapper_pg_dir, PGSIZE);                                                                                   
        
            //load elf program++++++++++++++++++++++++++++++
                //
                    load_program(task[i]);
                //
            //end load elf program+++++++++++++++++++++++++++

            //Or Or Or Or Or Or Or 

            //load simple program+++++++++++++++++++++++++++
            //
                //Attention: `alloc` create virtual address(just constant offset) space for user space
                //copy uapp to user space
                // uint64_t* uapp_copyed = (uint64_t*)alloc_pages((((uint64_t)&_eramdisk - (uint64_t)&_sramdisk) / PGSIZE) + 1);
                // memcpy((void*)uapp_copyed, (void*)&_sramdisk, (uint64_t)&_eramdisk - (uint64_t)&_sramdisk);
                // create_mapping(task[i]->pgd, USER_START, (uint64_t)uapp_copyed - PA2VA_OFFSET, (uint64_t)&_eramdisk - (uint64_t)&_sramdisk, 0x1f);//U│X│W│R│V
            //
            //end load simple program+++++++++++++++++++++++++++
        //new stack for user space
        //Size of stack is PGSIZE
        uint64_t* new_stack = (uint64_t*)alloc_page();
        create_mapping(task[i]->pgd, USER_END - PGSIZE, (uint64_t)new_stack - PA2VA_OFFSET, PGSIZE, 0x17);//U│W│R│V
        //end added in Lab4---------------------------------------------
    }

    printk("...task_init done!\n");
}

void switch_to(struct task_struct *next) {
    printk("\n");
    if(current->pid != next->pid) {
        printk("switch to [PID = %d PRIORITY = %d COUNTER = %d]\n", next->pid, next->priority, next->counter);
        struct task_struct *prev = current;
        current = next;
        __switch_to(prev, next);
    }
}

void do_timer() {
    if(current->pid == idle->pid || current->counter == 0) {
        schedule();
        return;
    } else {
        current->counter--;
    }
    if(current->counter <= 0) {
        schedule();
    }
}

void schedule() {
    while(1){
        uint64_t max = 0;
        struct task_struct* max_task = NULL;
        for(int i = 0; i < NR_TASKS; i++) {
            if(task[i]->counter > max) {
                max = task[i]->counter;
                max_task = task[i];
            }
        }
        if(max == 0){
            for(int i = 1; i < NR_TASKS; i++) {
                task[i]->counter = task[i]->priority;
                printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", task[i]->pid, task[i]->priority, task[i]->counter);
            }
            continue;
        } else {
            switch_to(max_task);
            return;
        }
    }
}

#if TEST_SCHED
#define MAX_OUTPUT ((NR_TASKS - 1) * 10)
char tasks_output[MAX_OUTPUT];
int tasks_output_index = 0;
char expected_output[] = "2222222222111111133334222222222211111113";
#include "sbi.h"
#endif

void dummy() {
    uint64_t MOD = 1000000007;
    uint64_t auto_inc_local_var = 0;
    int last_counter = -1;
    while (1) {
        if ((last_counter == -1 || current->counter != last_counter) && current->counter > 0) {
            if (current->counter == 1) {
                --(current->counter);   // forced the counter to be zero if this thread is going to be scheduled
            }                           // in case that the new counter is also 1, leading the information not printed.
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
            #if TEST_SCHED
            tasks_output[tasks_output_index++] = current->pid + '0';
            if (tasks_output_index == MAX_OUTPUT) {
                for (int i = 0; i < MAX_OUTPUT; ++i) {
                    if (tasks_output[i] != expected_output[i]) {
                        printk("\033[31mTest failed!\033[0m\n");
                        printk("\033[31m    Expected: %s\033[0m\n", expected_output);
                        printk("\033[31m    Got:      %s\033[0m\n", tasks_output);
                        sbi_system_reset(SBI_SRST_RESET_TYPE_SHUTDOWN, SBI_SRST_RESET_REASON_NONE);
                    }
                }
                printk("\033[32mTest passed!\033[0m\n");
                printk("\033[32m    Output: %s\033[0m\n", expected_output);
                sbi_system_reset(SBI_SRST_RESET_TYPE_SHUTDOWN, SBI_SRST_RESET_REASON_NONE);
            }
            #endif
        }
    }
}
