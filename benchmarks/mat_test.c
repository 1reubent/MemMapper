#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "../my_vm.h"


#define PAGE_SIZE 1<<13
#define LOOP_NUM 50
#define BUF_SIZE 5 //in bytes. num of chars in array

#define R1 4 // number of rows in M1
#define C1 2 // number of columns in M1
#define R2 2 // number of rows in M2
#define C2 4 // number of columns in M2

int main(int argc, char **argv)
{
    printf("Setting physical memory...\n");
    set_physical_mem();
    printf("\n");

    //MATRIX A
    printf("mallocing first matrix...\n");
    int m1Size = R1*C1;
    void* m1 = t_malloc(m1Size*sizeof(int*));
    printf("\n");
    for(int i=0;i<R1;i++){
        void* m1_curr_row = m1+(i*C1*sizeof(int));
        for(int j=0;j<C1;j++){
            int val = (i+j)+1;
            if(put_value((unsigned int)(uintptr_t)(m1_curr_row+(j*sizeof(int))),&val, sizeof(int)) == -1){
                perror("error\n");
                exit(1);
            }
        }
    }

    //MATRIX B
    printf("mallocing second matrix...\n");
    int m2Size = R2*C2;
    void* m2 = t_malloc(m2Size*sizeof(int));
    printf("\n");
    for(int i=0;i<R2;i++){
        void* m2_curr_row = m2+(i*C2*sizeof(int));
        for(int j=0;j<C2;j++){
            //m2[i][j] = (i*j)+1;
            int val = (i+j)+2;
            if(put_value((unsigned int)(uintptr_t)(m2_curr_row+(j*sizeof(int))),&val, sizeof(int)) == -1){
                perror("error\n");
                exit(1);
            }
        }
    }

    //INIT MATRIX C
    printf("mallocing result matrix...\n");
    int m3Size = R1*C2;
    void* m3 = t_malloc(m3Size*sizeof(int));
    printf("\n");


    //CHECK IF M1 AND M2 CAN BE MULTIPLIED TOGETHER
    if (C1 != R2) {
        perror("Number of Cols in M1 should equal number of rows in M2 (C1==R2)\n"); 
        exit(1);
    }
 
    //MULTIPLY MATRICIES
    mat_mult((unsigned int)(uintptr_t)m1, (unsigned int)(uintptr_t)m2, (unsigned int)(uintptr_t)m3, R1, C1, C2);
 
    printf("\n");
    print_TLB_missrate();
    return 0;
}
