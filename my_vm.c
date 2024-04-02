#include "my_vm.h"
#include <math.h>

#define PAGE_SIZE 1 * 8192 // Use a multiple of 8KB
#define PTE_SIZE 4 //num of bytes in 1 pte. should be a factor of page size

//static char memory[MEMSIZE];
char *topLevelPT; // = (char *) memory;
int numOffsetBits = 0;
int numOuterBits = 0;
int numInnerBits = 0;
int numIrrelaventBits = 0;
int firstTime =0;

char* vBitmap = NULL;
char* pBitmap = NULL;
//do all TLB stuff after page tables are done

//TODO: Define static variables and structs, include headers, etc.

void set_physical_mem(){
    //TODO: Finish
    char* memory = malloc(MEMSIZE);
    char *topLevelPT = memory; // = (char *) memory;

    numOffsetBits = log2(PAGE_SIZE);

    //init top level page table; store in page 0 of phys. mem.
    
    //can top level page table be more than 1 page?

    //find number of outer page bits
    int numofPTEs = (MAX_MEMSIZE) / (PAGE_SIZE); //also the num of pages in VAS
    int numPagesforPT = ( (numofPTEs) * (PTE_SIZE) ) / (PAGE_SIZE); // this is the num of elements in top-level PT
    numOuterBits = log2(numPagesforPT);

    //find number of inner page bits
    int numOfElementsIn2ndPT = (PAGE_SIZE) / (PTE_SIZE);
    numInnerBits = log2(numOfElementsIn2ndPT);
    
    //if num of bits dont add up to 32
    if(numOffsetBits + numInnerBits + numOuterBits < 32){
        //extract relevant bits in VAs before translating them
        numIrrelaventBits = 32 - (numOffsetBits + numInnerBits + numOuterBits);
    }else if(numOffsetBits + numInnerBits + numOuterBits > 32){
        perror("error: invalid page size, memory size, or pte size");
        exit(1);
    }else{
        numIrrelaventBits = 0;
    }

    //init bitmaps
    vBitmap = malloc(numofPTEs/8); // char is 8 bits
    pBitmap = malloc( (MEMSIZE / PAGE_SIZE) /8);



}

void * translate(unsigned int vp){
    //TODO: Finish
    //given virtual adress, translate to physical addr

    //extract offset num from VA
    unsigned int mask = (1 << numOffsetBits) - 1;
    int offset = vp & mask;

    //extract inner page num
    unsigned mask = ((1 << numInnerBits) - 1) << numOffsetBits;
    int innerPageNum = vp & mask;
    
    //extract outer page num
    unsigned mask = ((1 << numOuterBits) - 1) << (numOffsetBits+numInnerBits);
    int outerPageNum = vp & mask;
    
    //CHECK TLB HERE

    //walk the tables


    //first check if VA is in TLB.  
        //If so, copy PA of physical page and concatonate offset bits of VA to it. return it.
    //else, use VA to walk through page tables and find PA
        //extract outer page # and go to that index in top-level page table. Go to that addr of second-level PT
        //extract inner page # and go to that index in second level page table. Save that addr of phys. mem.
        //concatonate offset bits to addr. of phys. page
        //add to TLB and return
}

unsigned int page_map(unsigned int vp){
    //TODO: Finish
}

void * t_malloc(size_t n){
    //TODO: Finish

    //return NULL if n is not a multiple of PAGE_SIZE
    if(firstTime=0){
        firstTime =1;
        set_physical_mem();
        
    }
}

int t_free(unsigned int vp, size_t n){
    //TODO: Finish
}

int put_value(unsigned int vp, void *val, size_t n){
    //TODO: Finish
}

int get_value(unsigned int vp, void *dst, size_t n){
    //TODO: Finish
}

void mat_mult(unsigned int a, unsigned int b, unsigned int c, size_t l, size_t m, size_t n){
    //TODO: Finish
}

void add_TLB(unsigned int vpage, unsigned int ppage){
    //TODO: Finish
}

int check_TLB(unsigned int vpage){
    //TODO: Finish
}

void print_TLB_missrate(){
    //TODO: Finish
}
