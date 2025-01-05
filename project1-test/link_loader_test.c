#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/syscall.h>  

#define SYS_my_get_physical_addresses  454 // copy from user

int a[2000000];

int main(){

    void *ptr1 = (void *)(&a[0]);
    void *ptr2 = (void *)(&a[1999999]);
    unsigned long ret1 = (void *)syscall(SYS_my_get_physical_addresses, &ptr1);
    unsigned long ret2 = (void *)syscall(SYS_my_get_physical_addresses, &ptr2);
    printf("global element a[0]:\n");  
    printf("Offset of logical address:[%p]   Physical address:[%p]\n", &a[0], ptr1);              
    printf("========================================================================\n"); 
    printf("global element a[1999999]:\n");  
    printf("Offset of logical address:[%p]   Physical address:[%p]\n", &a[1999999], ptr2);              
    printf("========================================================================\n"); 

    return 0;
}

