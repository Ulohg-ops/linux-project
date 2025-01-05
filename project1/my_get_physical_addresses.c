#include <linux/kernel.h>     
#include <linux/syscalls.h>  
#include <linux/uaccess.h>     
#include <linux/mm.h>         
#include <linux/highmem.h>   
#include <linux/sched.h>     

SYSCALL_DEFINE1(my_get_physical_addresses, void __user *, usr_ptr) {
    unsigned long vaddr;     
    unsigned long paddr = 0;
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    struct page *page;
    
    if (copy_from_user(&vaddr, usr_ptr, sizeof(vaddr))) {
        pr_err("Failed for copy_from_user\n");
        return -EFAULT;
    }

    pr_info("Received virtual address: 0x%lx\n", vaddr);

    pgd = pgd_offset(current->mm, vaddr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        pr_err("Invalid PGD\n");
        return -EFAULT;
    }

    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        pr_err("Invalid P4D\n");
        return -EFAULT;
    }

    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud) || pud_bad(*pud)) {
        pr_err("Invalid PUD\n");
        return -EFAULT;
    }

    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        pr_err("Invalid PMD\n");
        return -EFAULT;
    }

    pte = pte_offset_map(pmd, vaddr);
    if (!pte || !pte_present(*pte)) {
        pr_err("Invalid or non-present PTE\n");
        if (pte) pte_unmap(pte); 
        return -EFAULT;
    }

    page = pte_page(*pte);
    if (!page) {
        pr_err("Failed to get page\n");
        pte_unmap(pte);
        return -EFAULT;
    }

    paddr = (page_to_pfn(page) << PAGE_SHIFT) | (vaddr & ~PAGE_MASK);
    pte_unmap(pte);

    pr_info("Physical address: 0x%lx\n", paddr);

    if (copy_to_user(usr_ptr, &paddr, sizeof(paddr))) {
        pr_err("Failed to copy physical address to user\n");
        return -EFAULT;
    }

    return 0; 
}

