#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "../my_vm.h"


#define PAGE_SIZE 1<<13
#define LOOP_NUM 3 //num of mallocs to make
#define BUF_SIZE 5 //in bytes. num of chars in array

int main(int argc, char **argv)
{
    printf("Setting physical memory...\n");
    set_physical_mem();

    size_t n = (1 * PAGE_SIZE);
    void* ptrArr[LOOP_NUM];
    //malloc loop

    for (int i=0;i<LOOP_NUM;i++){
        printf("\n");
        printf("MALLOC #%d ...\n", i);
        ptrArr[i] = t_malloc(n);
        
        printf("TEST #%d ...\n", i);
        char test[BUF_SIZE] = {i + '0' ,i+1 + '0' ,i+2 + '0' ,i+3 + '0' ,'\0' };
            //last character must be null terminator -> \0

        printf("putting %d bytes: %s ...\n", BUF_SIZE, test);
        put_value((unsigned int)(uintptr_t)ptrArr[i], test, BUF_SIZE);

        
    }
    //free loop
    for (int i=0;i<LOOP_NUM;i++){
        printf("\n");

        char dest[BUF_SIZE];
        get_value((unsigned int)(uintptr_t)ptrArr[i], dest, BUF_SIZE);
        printf("got %d bytes: %s\n", BUF_SIZE, dest);

        
        printf("FREE #%d ...\n", i);
        t_free((unsigned int)(uintptr_t)ptrArr[i], n);
    }
    
    printf("WWWWWWWWWWWW\n");
    return 0;
}
