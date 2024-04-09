#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "../my_vm.h"


#define PAGE_SIZE 1<<15
#define LOOP_NUM 3
#define BUF_SIZE 5

int main(int argc, char **argv)
{
    printf("Setting physical memory...\n");
    set_physical_mem();

    size_t n = (1 * PAGE_SIZE);
    void* ptrArr[LOOP_NUM];
    //malloc loop
    for (int i=0;i<LOOP_NUM;i++){
        printf("MALLOC #%d ...\n", i);
        ptrArr[i] = t_malloc(n);
        
        printf("TEST #%d...\n", i);
        char test[BUF_SIZE] = {i + '0' ,i+1 + '0' ,i+2 + '0' ,i+3 + '0' ,i+4 + '0' };
        put_value(ptrArr[i], test, BUF_SIZE);
        get_value(ptrArr[i], test, BUF_SIZE);
    }
    //free loop
    for (int i=0;i<LOOP_NUM;i++){
        printf("FREE #%d ...\n", i);
        t_free((unsigned int)(uintptr_t)ptrArr[i], n);
    }
    // int id = 1;
    // worker_create(&thread, NULL, &dummy_work, &id);

    // printf("Main thread waiting\n");
    // worker_join(thread, NULL);
    // printf("Main thread resume\n");

    // printf("Main thread exit\n");
    printf("WWWWWWWWWWWW\n");
    return 0;
}
