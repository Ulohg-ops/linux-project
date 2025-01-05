#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/syscall.h>  
#include <sys/wait.h>  

#define SYS_my_get_physical_addresses 451

int global_a = 123;  

int main() {
    // void *parent_use, *child_use;
    void *ptr1 = (void *)(&global_a);
    unsigned long ret1;

    printf("********************** Before Fork ******************************\n");
    ret1 = (void *)syscall(SYS_my_get_physical_addresses, &ptr1);
    printf("pid=%d: Global variable global_a: %d\n", getpid(), global_a);
    printf("Logical address offset: [%p] Physical address: [%p]\n", &global_a, ptr1);
    printf("***************************************************************\n");
    ptr1 = (void *)(&global_a);//assign again
    if (fork() == 0) {  
        // Child process code
        printf("111111111111111111111 After Fork (Child Process) 111111111111111111111111\n");
        ret1 = (void *)syscall(SYS_my_get_physical_addresses, &ptr1);
        printf("pid=%d: Global variable global_a: %d\n", getpid(), global_a);
        printf("Logical address offset: [%p] Physical address: [%p]\n", &global_a, ptr1);
        printf("***************************************************************\n");
        ptr1 = (void *)(&global_a);//assign again
        
        // Trigger Copy-on-Write (CoW)
        global_a = 789;  
        printf("************ Test Copy-on-Write in Child Process **********************\n");
        ret1 = (void *)syscall(SYS_my_get_physical_addresses, &ptr1);
        printf("pid=%d: Global variable global_a: %d\n", getpid(), global_a);
        printf("Logical address offset: [%p] Physical address: [%p]\n", &global_a, ptr1);
        printf("***************************************************************\n");
        ptr1 = (void *)(&global_a);//assign again
        sleep(5);
    } else {  
        printf("vvvvvvvvvvvvvvvvvvvvvvv After Fork (Parent Process) vvvvvvvvvvvvvvvvvvvvv\n");
        ret1 = (void *)syscall(SYS_my_get_physical_addresses, &ptr1);
        printf("pid=%d: Global variable global_a: %d\n", getpid(), global_a);
        printf("Logical address offset: [%p] Physical address: [%p]\n", &global_a, ptr1);
        printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
        ptr1 = (void *)(&global_a);//assign again
        wait(NULL);
    }

    return 0;
}

