#include "my_vm.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define PAGE_SIZE 1 * 8192 // Use a multiple of 8KB
#define PTE_SIZE 4 //DO NOT CHANGE. num of bytes in 1 pte. should be a factor of page size


char *page0; //ptr to start of page 0 AND top level PT
int numOffsetBits = 0;
int numOuterBits = 0;
int numInnerBits = 0;
int numIrrelaventBits = 0; //bits in the 32-bit address that are unused. need to remove them before translating, they could be non-zero.
int firstTime =0;
int numOfPTEsInOnePage;

char* virtualBitmap = NULL;
int vBitmapSize = (MAX_MEMSIZE / PAGE_SIZE) /8; // num of chars in bitmap. char is 1 byte, 8 bits. Every 8 pages takes 1 char.

char* physicalBitmap = NULL;
int pBitmapSize = (MEMSIZE / PAGE_SIZE) /8;

//Design and other information:   
    //PTE is an int which is the physical page number its pointing to.
    //uninitialized PTE is set to -1.
    //location of a page is page0 + ( (page num) * (PAGE_SIZE) ). 
        //limit is this number + PAGE_SIZE
    //location of a PTE in a page is page_location + (PTE_index) * (PTE_SIZE)


//do all TLB stuff after page tables are done

//0 is virtualBitmap, 1 is physicalBitmap
//on success toStoreIndex will contain the next available page number
//on failure, get_next_avail will return -1.

//TODO: this function needs to retrun first UNSET bit, not first set bit. FIX
static int get_next_avail(int virtOrphys, unsigned int* toStoreIndex)     // unsigned int bc int might not be able to represent enough numbers.
{
    //need to convert char array to ints
    int numOfInts = (virtOrphys==0) ? vBitmapSize/4 : pBitmapSize/4; // Every 4 characters is an int
    char* bitmap = (virtOrphys==0) ? virtualBitmap : physicalBitmap;

    //create 4 char array to be casted as an int. FLIP all bits, so that first 'set' bit is actually the frist unset bit.
    char currInt[4];
    for(int i =0; i< numOfInts ; i++){
        currInt[0] = (bitmap[(4*i) + 0]==0)? 1: 0;
        currInt[1] = (bitmap[(4*i) + 1]==0)? 1: 0;
        currInt[2] = (bitmap[(4*i) + 2]==0)? 1: 0;
        currInt[3] = (bitmap[(4*i) + 3]==0)? 1: 0;

        //cast char array as int
        int* num = (int*) currInt;

        if ((*num) == 0) {
            //no bits are 1
            continue;
        }

        unsigned int index = (unsigned int)log2((*num) & -(*num));
        *toStoreIndex = index;
        return 0;//success
    }
    return -1; //fail
    
    
    
}

static void set_bit_at_index(char *bitmap, int index)
{    
    // Calculate the byte index and bit position within the byte
    int byte_index = index / 8;
    int bit_position = index % 8;

    // Set the bit at the specified index
    bitmap[byte_index] |= (1 << bit_position);
    return;
}

void set_physical_mem(){
    char* memory = calloc(MEMSIZE); //calloc so it starts at 0.
    page0 = memory;

    //find num of offset bits
    numOffsetBits = log2(PAGE_SIZE);

    //init top level page table; store in page 0 of phys. mem.
    

    //find number of outer page bits
        // int numofPTEs = (MAX_MEMSIZE) / (PAGE_SIZE); //also the num of pages in VAS
        // int numPagesforPT = ( (numofPTEs) * (PTE_SIZE) ) / (PAGE_SIZE); // this is the num of elements in top-level PT
    numOfPTEsInOnePage = (PAGE_SIZE) / (PTE_SIZE);
    numOuterBits = log2(numOfPTEsInOnePage);

    //find number of inner page bits
        //numOfPTEsIn2ndLevel = (PAGE_SIZE) / (PTE_SIZE);
    numInnerBits = log2(numOfPTEsInOnePage);
    
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
    virtualBitmap = calloc(vBitmapSize); 
        //set bitmap to 0;
   

    physicalBitmap = calloc(pBitmapSize);
        //set bitmap to 0;

    //init top page
    set_bit_at_index(physicalBitmap, 0);
        //set each PTE in top page to -1 aka uninitialized.
    for(int i =0; i< numOfPTEsInOnePage; i++){
        *(page0 + (i * PTE_SIZE)) = -1;
    }



}

//walks the page table. If any PTE is uninitialized, pageTableWalker will return the address of that PTE.
    //if it's less than page0+PAGE_SIZE means top level PTE is uninit-ed
    //if it's greater than page0+PAGE_SIZE means second level PTE is unit-ed
    //set toInit to NULL before calling pageTableWalker() if it doesnt't matter (ie for translate())

//if page_map() is the caller, toStorePhysicalPageNum should not be NULL. toStorePhysicalPageNum will then store the page number of the physical address and pageTableWalker will return NULL

//for translate(), on success, pageTableWalker() should return a ptr to the data in physical memory
//for page_map(), on success, pageTableWalkr() should return NULL and toStorePhysicalPageNum should contain the physical page number.
void* pageTableWalker(int topLevelIndex, int secondLevelIndex, int offset, int* toStorePhysicalPageNum){
    //search top level for second level page number
    int* topLevelPTELocation = (int*) ( page0 + ( topLevelIndex * PTE_SIZE) );
    int secondLevelPageNum = *topLevelPTELocation;
        if(secondLevelPageNum == -1){ 
            if(toStorePhysicalPageNum!=NULL){
                //page map is caller
                //*toInit = topLevelPTELocation;
                return topLevelPTELocation;
            }
            return NULL;
        }

    //search second level for physical page number
    int* secondLevelPTELocation = (int*) ( ( page0 + ( secondLevelPageNum * PAGE_SIZE) ) + (secondLevelIndex * PTE_SIZE) );
    int physicalPageNum = *secondLevelPTELocation;
        if(physicalPageNum == -1){
            if(toStorePhysicalPageNum!=NULL){
                //page_map is the caller. 
                //*toInit = secondLevelPTELocation;
                return secondLevelPTELocation;
            }
            return NULL;
        }else{
            if(toStorePhysicalPageNum!=NULL){
                //page_map is the caller. it only needs the physical page number.
                *toStorePhysicalPageNum = physicalPageNum;
                return NULL;
            }
        }

    //get physical address 
    char* data = (char*) ( (page0 + (physicalPageNum * PAGE_SIZE)) + offset );
    return data;

}
void * translate(unsigned int vp){

    //given virtual adress, translate to physical addr

    //CHECK TLB HERE
    //else walk the tables

    //extract offset num from VA
    unsigned int mask = (1 << numOffsetBits) - 1;
    int offset = vp & mask;

    //extract inner page num
    unsigned mask = ((1 << numInnerBits) - 1) << numOffsetBits;
    int innerPageNum = vp & mask;
    
    //extract outer page num
    unsigned mask = ((1 << numOuterBits) - 1) << (numOffsetBits+numInnerBits);
    int outerPageNum = vp & mask;
    
    

    
    void* physicalAddress = pageTableWalker(outerPageNum, innerPageNum, offset, NULL);
    if (physicalAddress ==NULL){
        return NULL;
    }
    return physicalAddress;


    //first check if VA is in TLB.  
        //If so, copy PA of physical page and concatonate offset bits of VA to it. return it.
    //else, use VA to walk through page tables and find PA
        //extract outer page # and go to that index in top-level page table. Go to that addr of second-level PT
        //extract inner page # and go to that index in second level page table. Save that addr of phys. mem.
        //concatonate offset bits to addr. of phys. page
        //add to TLB and return
}

unsigned int page_map(unsigned int vp){
    //walk the tables   

    //extract offset num from VA
    unsigned int mask = (1 << numOffsetBits) - 1;
    int offset = vp & mask;

    //extract inner page num
    unsigned mask = ((1 << numInnerBits) - 1) << numOffsetBits;
    int innerPageNum = vp & mask;
    
    //extract outer page num
    unsigned mask = ((1 << numOuterBits) - 1) << (numOffsetBits+numInnerBits);
    int outerPageNum = vp & mask;

    int physicalPageNum;
    int* physicalPageNumPtr = &physicalPageNum;
    void* toInit = pageTableWalker(outerPageNum, innerPageNum, offset, physicalPageNumPtr);
    if(toInit !=NULL){
        //need to allocate some page(s)

        //if it's less than page0+PAGE_SIZE means top level PTE is uninit-ed
        //if it's greater than page0+PAGE_SIZE means second level PTE is unit-ed
        int* uninitPTEAddress = (int*) toInit;
        if(uninitPTEAddress < page0+PAGE_SIZE){
            //need to allocate top-level PTE's page
                //search physicalBitmap for open page for second-level page table
                    //if not error
                //set that bit, and set *uninitedPTEAddress to that page number
                //uninitedPTEAddress = address of new second-level page table

            //int newPageNum;
            //int* newPageNumPtr = &newPageNum;
            int status = get_next_avail(1, physicalPageNumPtr);
            if(status == -1){
                perror("not enough space ");
                exit(1);
            }
            set_bit_at_index(physicalBitmap, physicalPageNum);
            *uninitPTEAddress = page0+ (physicalPageNum*PAGE_SIZE); 
            uninitPTEAddress = page0+ (physicalPageNum*PAGE_SIZE); //set uninitPTEAddress to address of second-level PTE here to init second-level PTE in next if-statement.
        }
        if(uninitPTEAddress >= page0+PAGE_SIZE){
            //need to allocate second-level PTE's page
            //int newPageNum;
            //int* newPageNumPtr = &newPageNum;
            int status = get_next_avail(1, physicalPageNumPtr);
            if(status == -1){
                perror("not enough space ");
                exit(1);
            }
            set_bit_at_index(physicalBitmap, physicalPageNum);
            *uninitPTEAddress = page0+ (physicalPageNum*PAGE_SIZE); 
        }

    }

    //Got physical page number by this point
    return physicalPageNum;

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
