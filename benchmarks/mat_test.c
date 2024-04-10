#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "../my_vm.h"


#define PAGE_SIZE 1<<15
#define LOOP_NUM 3
#define BUF_SIZE 5 //in bytes. num of chars in array

#define R1 2 // number of rows in Matrix-1
#define C1 2 // number of columns in Matrix-1
#define R2 2 // number of rows in Matrix-2
#define C2 3 // number of columns in Matrix-2

int main(int argc, char **argv)
{
    printf("Setting physical memory...\n");
    set_physical_mem();


    // R1 = 4, C1 = 4 and R2 = 4, C2 = 4 (Update these
    // values in MACROs)

    //MATRIX A
    int m1Size = R1*C1;
    int** m1 = t_malloc(m1Size*sizeof(int));
    for(int i=0;i<R1;i++){
        m1[i] = t_malloc(C1*sizeof(int));
        for(int j=0;j<C1;j++){
            m1[i][j] = i*j;
        }
    }

    //MATRIX B
    int m2Size = R2*C2;
    int** m2 = t_malloc(m2Size*sizeof(int));
    for(int i=0;i<R2;i++){
        m2[i] = t_malloc(C2*sizeof(int));
        for(int j=0;j<C2;j++){
            m2[i][j] = (i*j)+1;
        }
    }

    //INIT MATRIX C
    int m3Size = R1*C2;
    int** m3 = t_malloc(m3Size*sizeof(int));
    for(int i=0;i<R1;i++){
        m3[i] = t_malloc(C2*sizeof(int));
    }


    // if coloumn of m1 not equal to rows of m2
    if (C1 != R2) {
        printf("The number of columns in Matrix-1 must be "
               "equal to the number of rows in "
               "Matrix-2\n");
        printf("Please update MACROs value according to "
               "your array dimension in "
               "#define section\n");
 
        exit(1);
    }
 
    // Function call
    mat_mult(m1, m2, m3, R1, C1, C2);
 
   
    printf("WWWWWWWWWWWW\n");
    return 0;
}
