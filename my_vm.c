#include "my_vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define PAGE_SIZE 1 * 8192 // Use a multiple of 8KB
#define PTE_SIZE 4 //DO NOT CHANGE. num of bytes in 1 pte. should be a factor of page size


char *page0; //ptr to start of page 0 AND top level PT
int* page0Int;

int numOffsetBits = 0;
int numOuterBits = 0;
int numInnerBits = 0;
int numIrrelaventBits = 0; //bits in the 32-bit address that are unused. need to remove them before translating, they could be non-zero.
int firstTime =0;
int numOfPTEsInOnePage;

char* virtualBitmap = NULL;
unsigned long vBitmapSize = (MAX_MEMSIZE / PAGE_SIZE) /8; // num of chars in bitmap. char is 1 byte, 8 bits. Every 8 pages takes 1 char.

char* physicalBitmap = NULL;
unsigned long pBitmapSize = (MEMSIZE / PAGE_SIZE) /8;

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
    //if checking virtualBitmap, you can check for multiple pages in one call to get_next_avail.
static int get_next_avail(int virtOrphys, unsigned int* toStoreIndex, int numOfPagesToAlloc)     // unsigned int bc int might not be able to represent enough numbers.
{
    //need to convert char array to ints

    //unsigned long numOfInts = (virtOrphys==0) ? vBitmapSize/4 : pBitmapSize/4; // Every 4 characters is an int
    //char* bitmap = (virtOrphys==0) ? virtualBitmap : physicalBitmap;

    //FLIP all bits, so that first 'set' bit is actually the frist unset bit.
        //cast char array as int
    int* num = (virtOrphys==0) ? (int*) virtualBitmap : (int*) physicalBitmap;
    int numFlip = ~(*num);
    if (numFlip == 0) {
        //no bits are 1
        return -1; //fail
    }
    if(virtOrphys==0 && numOfPagesToAlloc>1){
        //looking thru virtual bitmap for more than 1 page, so they need to be consecutive/contiguous. need to use a loop.
        //store index of firstly allcoated page in toStoreIndex
        
        int count = 0;
        int index= 0;
        while (numFlip > 0) {
            if ((numFlip & 1) == 1) {
                //found a set bit
                if(count==0){
                    //if first set bit, save this index in case it needs to be returned.
                    *toStoreIndex = index;
                }
                count++;
                if (count == numOfPagesToAlloc)
                    return 0; //Found all consecutive set bits
            } else {
                count = 0; //Reset count if bit is not set
            }
            numFlip >>= 1; //Right shift to check the next bit
            index++; 
        }
        return -1; // fail. no consecutive open pages.

        
    }
    unsigned int index = (unsigned int)__builtin_ffs((numFlip) & ( (*num) + 1));
        //these operations shouldn't change the actual bitmap right? TODO FIX
    *toStoreIndex = index;
    return 0;//success

    
    
    
}

static void set_bit_at_index(char *bitmap, unsigned int index, int numOfPagesToAlloc)
{    
    // Calculate the byte index and bit position within the byte
    for(int i=0;i<numOfPagesToAlloc;i++){
        unsigned int byte_index = (index+i) / 8;
        unsigned int bit_position = (index+i) % 8;

        // Set the bit at the specified index
        bitmap[byte_index] |= (1 << bit_position);
    }
    return;
}

void set_physical_mem(){
    char* memory = calloc(MEMSIZE, 1); //calloc so it starts at 0.
    page0 = memory;
    page0Int = (int*) memory;

    //find num of offset bits
    numOffsetBits = __builtin_ffs(PAGE_SIZE);

    //init top level page table; store in page 0 of phys. mem.
    

    //find number of outer page bits
        // int numofPTEs = (MAX_MEMSIZE) / (PAGE_SIZE); //also the num of pages in VAS
        // int numPagesforPT = ( (numofPTEs) * (PTE_SIZE) ) / (PAGE_SIZE); // this is the num of elements in top-level PT
    numOfPTEsInOnePage = (PAGE_SIZE) / (PTE_SIZE);
    numOuterBits = __builtin_ffs(numOfPTEsInOnePage);

    //find number of inner page bits
        //numOfPTEsIn2ndLevel = (PAGE_SIZE) / (PTE_SIZE);
    numInnerBits = __builtin_ffs(numOfPTEsInOnePage);
    
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
    virtualBitmap = calloc(vBitmapSize, 1); 
        //set bitmap to 0;
   

    physicalBitmap = calloc(pBitmapSize, 1);
        //set bitmap to 0;

    //init top page
    set_bit_at_index(physicalBitmap, 0, 1);
        //set each PTE in top page to -1 aka uninitialized.
    for(int i =0; i< numOfPTEsInOnePage; i++){
        * (page0Int + (i * PTE_SIZE)) = -1;
    }



}

//walks the page table. If any PTE is uninitialized, pageTableWalker will return the address of that PTE.
    //if it's less than page0+PAGE_SIZE means top level PTE is uninit-ed
    //if it's greater than page0+PAGE_SIZE means second level PTE is unit-ed
//set toStorePhysicalPageNum to NULL before calling pageTableWalker() if it doesn't matter (ie for translate())
//if page_map() is the caller, toStorePhysicalPageNum should NOT be NULL. toStorePhysicalPageNum will then store the page number of the physical address and pageTableWalker will return NULL

//for translate(), on success, pageTableWalker() should return a ptr to the data in physical memory
//for page_map(), on success, pageTableWalkr() should return NULL and toStorePhysicalPageNum should contain the physical page number.
void* pageTableWalker(int topLevelIndex, int secondLevelIndex, int offset, unsigned int* toStorePhysicalPageNum){
    //search top level for second level page number
    int* topLevelPTELocation = (int*) ( page0Int + ( topLevelIndex * PTE_SIZE) );
    int secondLevelPageNum = *topLevelPTELocation;
        if(secondLevelPageNum == -1){ 
            //PTE is unititialized
            if(toStorePhysicalPageNum!=NULL){
                //page map is caller
                //*toInit = topLevelPTELocation;
                return topLevelPTELocation;
            }
            return NULL;
        }

    //search second level for physical page number
    int* secondLevelPTELocation = (int*) ( ( page0Int + ( secondLevelPageNum * PAGE_SIZE) ) + (secondLevelIndex * PTE_SIZE) );
    int physicalPageNum = *secondLevelPTELocation;
        if(physicalPageNum == -1){
            //PTE is uninitialized
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
    char* physicalAddress = (char*) ( (page0Int + (physicalPageNum * PAGE_SIZE)) + offset );
    return physicalAddress;

}
void * translate(unsigned int vp){

    //given virtual adress, translate to physical addr

    //CHECK TLB HERE, FIX TODO
    //else walk the tables

    //extract offset num from VA
    unsigned int mask = (1 << numOffsetBits) - 1;
    int offset = vp & mask;

    //extract inner page num
    mask = ((1 << numInnerBits) - 1) << numOffsetBits;
    int innerPageNum = vp & mask;
    
    //extract outer page num
    mask = ((1 << numOuterBits) - 1) << (numOffsetBits+numInnerBits);
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
    mask = ((1 << numInnerBits) - 1) << numOffsetBits;
    int innerPageNum = vp & mask;
    
    //extract outer page num
    mask = ((1 << numOuterBits) - 1) << (numOffsetBits+numInnerBits);
    int outerPageNum = vp & mask;

    unsigned int physicalPageNum;
    unsigned int* physicalPageNumPtr = &physicalPageNum;
    void* toInit = pageTableWalker(outerPageNum, innerPageNum, offset, physicalPageNumPtr);
    if(toInit !=NULL){
        //need to allocate some page(s). toInit holds the address of the uninitialized PTE

        //if toInit less than page0+PAGE_SIZE means top level PTE is uninit-ed
        //if toInit greater than or equal to page0+PAGE_SIZE means second level PTE is unit-ed
        int* uninitPTEAddress = (int*) toInit;
        if(uninitPTEAddress < page0Int + PAGE_SIZE){
            //need to allocate top-level PTE's page
                //search physicalBitmap for open page for second-level page table
                    //if not error
                //set that bit, and set *uninitedPTEAddress to that page number
                //uninitedPTEAddress = address of new second-level page table

            int status = get_next_avail(1, physicalPageNumPtr, 1);
            if(status == -1){
                perror("not enough space in physical memory");
                exit(1);
            }
            set_bit_at_index(physicalBitmap, physicalPageNum, 1);
            *uninitPTEAddress = physicalPageNum; //PTE should store physical page number of the page we just allocated
            uninitPTEAddress = page0Int+ (physicalPageNum*PAGE_SIZE); //set uninitPTEAddress to address of second-level PTE, to init second-level PTE in the following if-statement.
        }
        if(uninitPTEAddress >= page0Int+PAGE_SIZE){
            //need to allocate second-level PTE's page
            
            int status = get_next_avail(1, physicalPageNumPtr, 1);
            if(status == -1){
                perror("not enough space in physical memory");
                exit(1);
            }
            set_bit_at_index(physicalBitmap, physicalPageNum, 1);
            *uninitPTEAddress = physicalPageNum;
        }

    }

    //Got physical page number by this point
    return physicalPageNum;

}

void * t_malloc(size_t n){
    //TODO: Finish

    //find num of pages to be allocated

    //find free page(s) in virtual bitmap. (should be contiguous)
        //need to generate virtual address to return to user.
        //should be the starting address of the first page they allocated IN VIRTUAL memory.
        //that's just (first_allocated_page_num) * (PAGE_SIZE)
    
    //find free page(s) in physical bitmap
    //traverse page table to allocate physical page(s) (set to physical page num(s) determined thru physical bitmap)
        //use virtual address(es) (previously determined) to traverse PT
        //use page_map()
        

    //remember irrelavent bits

    if(firstTime==0){
        firstTime =1;
        set_physical_mem();
    }

    int numOfPagesToAlloc = (int) ceil(((double) n) / PAGE_SIZE); //min num of pages needed
    ;
    //allocate virtual page(s)

    //need to find a contiguous set of pages
    unsigned int index;
    unsigned int* toStoreIndex = &index;
    //get and set bits in virtual bitmap
    if(get_next_avail(0,toStoreIndex, numOfPagesToAlloc) == -1){
        perror("not enough space in virtual memory");
        exit(1);
    }
    //need to set all of the virtual bits at once. FIX TODO
    set_bit_at_index(virtualBitmap, index, numOfPagesToAlloc);

    //generate virtual address for this virtual page
        //untptr_t allows you to cast an int as a void*
    unsigned int virtualAddressUnsigned = ((index) * PAGE_SIZE);
    void* virtualAddressToReturn  =(void*)(uintptr_t)virtualAddressUnsigned; //need to return starting address of first page to user

    //allocate physical page(s)
    for(int i=0; i<numOfPagesToAlloc ;i++){
        //for each page the user allocated in virtual memory, map/alocate physical page

        int physicalPageNum = page_map(virtualAddressUnsigned + (i*PAGE_SIZE)); //map every virtual page that was allcoated. they're contiguous in VM, so just increment by page size.
    }
    //FIX TOD0: add PTE to TLB???

    //returns virtual address
    return virtualAddressToReturn;
}

int t_free(unsigned int vp, size_t n){
    //TODO: Finish

    /*ALGROITHM & NOTES*/

    //vp should always be the start of a page (check for that)
    //if n bleeds onto multiple pages, free them too. same ceil() func as in t_malloc()
    

    //for each page
        //calculate the starting address of the virtual page (for the first page its just vp, for the rest add i*PAGE_SIZE)
        //unset the virtual bit
            // index = vp/PAGE_SIZE
        //use vp address to uninitialize PTEs (walk them manually, w/o pageTableWalker):
            //traverse page tables, obtain physical page number to use later (unsetting physical bit)
            //uninitialize the second-level PTE
            //uninitialize the top-level PTE
        //remove PTE from TLB if it's in there FIX TODO
        //unset the physical bit in bitmap (use previously obtained page # as index)

    if(vp% PAGE_SIZE != 0 || vp+n >= MAX_MEMSIZE){
        //not an appropriate virtual address OR size.
        return -1;
    }
    //find num pages to be freed
    int numOfPagesToFree = (int) ceil(((double) n) / PAGE_SIZE);

    /*FOR EACH PAGE TO BE FREED...*/
    for(int i=0; i<numOfPagesToFree; i++){
        unsigned int vPageStartingAddress = vp + (i*PAGE_SIZE);
        unsigned int virtualPageNum = vPageStartingAddress / PAGE_SIZE;
            //this is also the index of the page in virtual bitmap
        
        /*CHECK IF PAGE IS ALREADY FREED*/
        int oneMask = 1 << virtualPageNum;
        if((*((int*)virtualBitmap) & oneMask) == 0){
            //page is already freed
            perror("double free on virtual page");
            exit(1);
        }

        /*UNSET VIRTUAL BIT*/
        unsigned int byte_index = (virtualPageNum) / 8;
        unsigned int bit_position = (virtualPageNum) % 8;
        // unset the bit at the specified index
        virtualBitmap[byte_index] &= ~(1 << bit_position);

        /*UNITINITIALIZE TOP AND SECOND LEVEL PTEs*/
        //extract offset num from VA
        unsigned int mask = (1 << numOffsetBits) - 1;
            int offset = vp & mask;
        //extract inner page num
        mask = ((1 << numInnerBits) - 1) << numOffsetBits;
            int innerPageNum = vp & mask;
        //extract outer page num
        mask = ((1 << numOuterBits) - 1) << (numOffsetBits+numInnerBits);
            int outerPageNum = vp & mask;
        
        int* topLevelPTELocation = (int*) ( page0Int + ( outerPageNum * PTE_SIZE) );
            int secondLevelPageNum = *topLevelPTELocation;
        int* secondLevelPTELocation = (int*) ( ( page0Int + ( secondLevelPageNum * PAGE_SIZE) ) + (innerPageNum * PTE_SIZE) );
            int physicalPageNum = *secondLevelPTELocation;
            //use later to uset physical bit
        
        //uninitialize second-level and top-level PTEs
        *secondLevelPTELocation = -1;
        *topLevelPTELocation = -1;

        /*FIX TODO: remove PTE from TLB if it's there*/


        /*CHECK IF PHYSICAL PAGE IS ALREADY FREED*/
        oneMask = 1<<physicalPageNum;
        if((*((int*)physicalBitmap) & oneMask) == 0){
            //page is already freed
            perror("double free on physical page");
            exit(1);
        }

        /*UNSET PHYSICAL BITMAP*/
        byte_index = (physicalPageNum) / 8;
        bit_position = (physicalPageNum) % 8;
        // unset the bit at the specified index
        physicalBitmap[byte_index] &= ~(1 << bit_position);
    }
    return 0; 
}

// int put_value(unsigned int vp, void *val, size_t n){
//     //TODO: Finish
// }

// int get_value(unsigned int vp, void *dst, size_t n){
//     //TODO: Finish
// }

// void mat_mult(unsigned int a, unsigned int b, unsigned int c, size_t l, size_t m, size_t n){
//     //TODO: Finish
// }

// void add_TLB(unsigned int vpage, unsigned int ppage){
//     //TODO: Finish
// }

// int check_TLB(unsigned int vpage){
//     //TODO: Finish
// }

// void print_TLB_missrate(){
//     //TODO: Finish
// }
