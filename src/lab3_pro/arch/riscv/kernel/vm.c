#include <vm.h>
/* early_pgtbl: 用于 setup_vm 进行 1GiB 的映射 */
uint64_t early_pgtbl[512] __attribute__((__aligned__(0x1000)));
void setup_vm() {
    /* 
     * 1. 由于是进行 1GiB 的映射，这里不需要使用多级页表 
     * 2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
     *     high bit 可以忽略
     *     中间 9 bit 作为 early_pgtbl 的 index
     *     低 30 bit 作为页内偏移，这里注意到 30 = 9 + 9 + 12，即我们只使用根页表，根页表的每个 entry 都对应 1GiB 的区域
     * 3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    **/
   
    memset(early_pgtbl, 0x0, PGSIZE);
    // first mapping
    // early_pgtbl[(uint64_t)PHY_START>>30] = ((uint64_t)(PHY_START>>30)<<28) | (uint64_t)0xf;
    //second mapping
    early_pgtbl[((uint64_t)(PHY_START+PA2VA_OFFSET)>> 30) & 0x1ff] = ((uint64_t)(PHY_START>>30)<<28) | (uint64_t)0xf;
}


/* swapper_pg_dir: kernel pagetable 根目录，在 setup_vm_final 进行映射 */
uint64_t swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));
extern uint64_t* _stext;
extern uint64_t* _srodata;
extern uint64_t* _sdata;
extern uint64_t* _etext;
extern uint64_t* _erodata;
extern uint64_t* _edata;

/* 创建多级页表映射关系 */
/* 不要修改该接口的参数和返回值 */
void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, uint64_t perm) {
    /*
     * pgtbl 为根页表的基地址
     * va, pa 为需要映射的虚拟地址、物理地址
     * sz 为映射的大小，单位为字节
     * perm 为映射的权限（即页表项的低 8 位）
     * 
     * 创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
     * 可以使用 V bit 来判断页表项是否存在
    **/
   uint64_t va_end = va + sz;
   for(uint64_t temp_va = va;temp_va < va_end;temp_va+=PGSIZE){
        uint64_t vpn_2 = (temp_va >> 30) & 0x1ff;
        uint64_t vpn_1 = (temp_va >> 21) & 0x1ff;
        uint64_t vpn_0 = (temp_va >> 12) & 0x1ff;
        uint64_t pte_2 = pgtbl[vpn_2];
        uint64_t pte_1 ;
        uint64_t* pgtabl_1;
        uint64_t* pgtabl_0;
        if((pte_2 & 0x1) == 0x1){
            //second level page table exist
            pgtabl_1 = (uint64_t*)(((pte_2 & 0x003ffffffffffc00)<<2)+PA2VA_OFFSET);
        }else{
            pgtabl_1 = (uint64_t*)((((uint64_t)kalloc()-PA2VA_OFFSET)>>12)<<10);
            pgtbl[vpn_2] = (uint64_t)pgtabl_1 | 0x1;
            pgtabl_1 = (uint64_t*)((((uint64_t)pgtabl_1>>10)<<12) + PA2VA_OFFSET);
        }
        pte_1 = pgtabl_1[vpn_1];
        if((pte_1 & 0x1) == 0x1){
            //first level page table exist
            pgtabl_0 = (uint64_t*)(((pte_1 & 0x003ffffffffffc00)<<2) + PA2VA_OFFSET);
        }else{
            pgtabl_0 = (uint64_t*)((((uint64_t)kalloc()-PA2VA_OFFSET)>>12)<<10);
            pgtabl_1[vpn_1] = (uint64_t)pgtabl_0 | 0x1;
            pgtabl_0 = (uint64_t*)((((uint64_t)pgtabl_0>>10)<<12) + PA2VA_OFFSET);
        }
        pgtabl_0[vpn_0] = (((uint64_t)pa & 0x003ffffffffffc00)>>2) | perm;
        pa += PGSIZE;
   }
//    printk("finish vpn\n");
}

void setup_vm_final() {
    memset(swapper_pg_dir, 0x0, PGSIZE);

    // printk("_stext == %llx\n",(uint64_t)&_stext);
    // printk("_etext == %llx\n",&_etext);
    // printk("_srodata == %llx\n",&_srodata);
    // printk("_erodata == %llx\n",&_erodata);
    // printk("_sdata == %llx\n",&_sdata);
    // printk("_edata == %llx\n",&_edata);

    // No OpenSBI mapping required

    // mapping kernel text X|-|R|V
    create_mapping(swapper_pg_dir, (uint64_t)&_stext, ((uint64_t)(&_stext)-PA2VA_OFFSET), ((uint64_t)(&_srodata)-(uint64_t)(&_stext)) , 0xb);

    // mapping kernel rodata -|-|R|V
    create_mapping(swapper_pg_dir, (uint64_t)&_srodata, ((uint64_t)(&_srodata)-PA2VA_OFFSET), ((uint64_t)(&_sdata)-(uint64_t)(&_srodata)) , 0x3);

    // mapping other memory -|W|R|V
    create_mapping(swapper_pg_dir, (uint64_t)&_sdata, ((uint64_t)(&_sdata)-PA2VA_OFFSET), (uint64_t)(PHY_SIZE - ((uint64_t)(&_sdata)-(uint64_t)(&_stext))) , 0x7);

    // set satp with swapper_pg_dir
    // printk("finish 3_mapping\n");
    uint64_t temp_satp = (((uint64_t)swapper_pg_dir-PA2VA_OFFSET)>>12) | (uint64_t)(0x8000000000000000);
    csr_write(satp,temp_satp);
    // flush TLB
    asm volatile("sfence.vma zero, zero");
    return;
}

