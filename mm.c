/*
 * mm.c
 *
 * Name: Brook Azene, Ilesh Shrestha
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * Also, read the project PDF document carefully and in its entirety before beginning.
 *
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*
 * If you want to enable your debugging output and heap checker code,
 * uncomment the following line. Be sure not to have debugging enabled
 * in your final submission.
 */
// #define DEBUG

#ifdef DEBUG
/* When debugging is enabled, the underlying functions get called */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#else
/* When debugging is disabled, no code gets generated */
#define dbg_printf(...)
#define dbg_assert(...)
#endif /* DEBUG */

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mem_memset
#define memcpy mem_memcpy
#endif /* DRIVER */

/* What is the correct alignment? */
#define ALIGNMENT 16



// global heap pointer pointing to start address (fake/special 8 byte padding initially) 
static size_t* heapPoint = 0;


// static functions in place of macros to define constant terms needed in initialisation/size computation
static size_t headSize() {return (ALIGNMENT / 2);}
static size_t footSize() {return (ALIGNMENT / 2);}
static size_t minBlock() {return (headSize() + footSize() + ALIGNMENT);}


//static uint64_t MAX(uint64_t x, uint64_t y) { if (x>y){ return x;} return y;}

static size_t PACK(size_t size, size_t alloc) { return (size | alloc);}

static size_t GET(size_t* p) { return (*(size_t *) p);}

static size_t PUT(size_t* p, size_t val) {return (*p = val);}

static size_t GET_SIZE(void* p) {return (GET(p) & -2);}

static bool GET_ALLOC(void* p) { return (GET(p) & 0x1);}

static size_t* HDRP(void* p) { return (size_t*)(p - 1);}

static size_t* FTRP(void* p) { return (size_t*)((char*)p + *(HDRP(p)) - 8);}

static void* NEXT_BLKP(void* p) { return ((char*)p + *(HDRP(p)));}

//static void* NEXT_BLKP(void* p) { return ((size_t*)p + FTRP(p) + ALGIN);}

//static void* PREV_BLKP(void* p) { return ((size_t*)p - GET_SIZE((size_t*)p - (size_t) 2));}

//static size_t CHUNKSIZE() {return 1 << 12;}


// Prototype def. for helper functions inside malloc()
static void place(size_t *bp, size_t asize);
static size_t* find_fit(size_t size);
static size_t* extend_heap(size_t size);



/* rounds up to the nearest multiple of ALIGNMENT */
static size_t align(size_t x)
{
    return ALIGNMENT * ((x+ALIGNMENT-1)/ALIGNMENT);
}



/*
 * Initialize: returns false on error, true on success.
 */
bool mm_init(void)
{

    // Empty heap initialized with min size 32 bytes
    heapPoint = mem_sbrk(minBlock());

    // Checks for error message from mem_sbrk()
    if (heapPoint == ((void *)-1)) 
    {
        return false;   
    }

    // Actually create the initial three words = prologue header + prologue footer, epilogue header
    // prologue pro;
    // epilogue epi;

    memset(heapPoint, (((size_t) 0) | 1), 8); //unused
    heapPoint = heapPoint + ((size_t) 1);
    
    memset(heapPoint, (((size_t) 8) | 1), 8); //pro h
    heapPoint = heapPoint + ((size_t) 1);
    
    memset(heapPoint, (((size_t) 8) | 1), 8); //pro f
    heapPoint = heapPoint + (uint64_t) 1;
    
    memset(heapPoint, (((size_t) 0) | 1), 8);// epi
    
   
    
    return true;
}

/*
 * malloc
 */
void* malloc(size_t size)
{   
    //printf("1\n");
    size_t asize;
    //size_t extendsize;
    size_t* blockp;

    if (size == 0){
        return NULL;
    }
    
   
        asize = align(size + 16);
        //printf("ASIZE: %ld \n", size); 
    
    
    // check to see if there is a block that can
    
    blockp = find_fit(asize);
    //printf("bp: %p\n", blockp);
    if((blockp)!= NULL)
    {
    
        place(blockp, asize);
        return blockp;
    
    }
    //printf("7\n");
    //extendsize = MAX(asize, (size_t)CHUNKSIZE);
    //printf("8\n");
    // determining how much we need to extend by
    
    // printf("%p\n", mem_heap_lo());
    // printf("%p\n", mem_heap_hi());
    // printf("%lu\n", asize);
    char* newReturn = mem_heap_hi() + 1;
    blockp = extend_heap(asize);

    if ((blockp) == NULL){
    	return NULL;
    }
    place(blockp, asize);
    // printf("asjaskjkj: %p\n", blockp);
     //printf("%p\n", mem_heap_lo());
     //printf("%p\n", mem_heap_hi());
    // printf("2\n");
   // printf("2\n");
    //printf("new: %p\n", newReturn);
    return newReturn;

}




//helper functions
static void place(size_t *bp, size_t asize){
    size_t csize =*(HDRP(bp));
    // splitting a found block that is too big
    if ((csize - asize) >= 32) {
    	// set header and footer of new allocated block
    	//*bp = PACK(asize, 1); // header
    	//printf("bp = %p", bp);
    	//bp = bp + asize - 2;
    	//*bp = PACK(asize, 1) //
    	PUT(HDRP(bp), PACK(asize,1));
    	PUT(FTRP(bp), PACK(asize,1));
    	bp = NEXT_BLKP(bp);
    	//sets header of new free block
    	PUT(HDRP(bp), PACK(csize-asize, 0));
    	// tried ftrp of new free block got core dump error
    	//PUT(FTRP(bp), PACK(csize-asize, 0));
    	
    }
    else
    {
    
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
    
    }
}

static size_t* find_fit(size_t size){
    //finding the first fit
    //void* searchp;
    // loop through the heap
    void* temp = mem_heap_lo() + 32;
    void* end = mem_heap_hi();

    //printf("5\n");
    
    while (temp <= end-8)
    { 
    	//printf("3\n");
        if (*(HDRP(temp)) > 16)
        {
          //  printf("2\n");
            if (!GET_ALLOC(HDRP(temp)) && (size <= (*(HDRP(temp)))))
            {
    	    //    printf("3\n");
    	    	return temp;
    	    }
    	
        }
    temp = NEXT_BLKP(temp);
     
    }

    return NULL;

}

static size_t* extend_heap(size_t size){

    size_t* blockp;
    //size_t asize;
    //asize = align(size);
    //printf("4\n");
    char* temp; 
    temp = mem_heap_hi()-7;
    //printf("temp = %p\n", temp); // header
    //printf("5\n");
    blockp = mem_sbrk(size);
    //printf("blockp = %p\n", blockp); //payload
    //printf("6\n");
    // memsbrk fails check
    if (blockp == (void*) -1){
    	return NULL;
    
    }
    //printf("new hi:  %p \n", mem_heap_hi());

    *((size_t*)temp) = (size | 0); // header
    char* new_end = mem_heap_hi();
    new_end = new_end - 7;
   // printf("new_end = %p\n", new_end); //epi
    
    *((size_t*)new_end) = 1;
    *((size_t*)(new_end -1)) = size; 
    
    //*(size_t*(((char*)(new_end))-7)) = (1);//epi
    //*(size_t*(((char*)(new_end))-7) -1) = size | 0; //footer
    // make a new header with the newly allocated size and epilogue header
    // new wpilogue header is bc old becomes header of new block
    //PUT(HDRP(blockp), PACK(size, 0));//header
    //PUT(FTRP(blockp), PACK(size, 0));//footer
    //PUT(HDRP(temp), PACK(8, 1)); // epi
    return blockp;

}







/*
 * free
 */
void free(void* ptr)
{
    void *temp = HDRP(ptr);
    size_t size = *(HDRP(temp));
    // bool check = in_heap((const void *) ptr);
    // size_t checAlloc = *(size_t *)(temp) & 0x1;


    // if ptr address provided is NULL/invalid
    if (ptr == NULL)
    {
        return;
    }
    // checks if ptr given is even within heap memory range, prints error for debugging
    // else if (!check)
    // {
    //     printf("\nError, POINTER NOT IN HEAP!!\n");
    // }
    // if ptr address is valid but block being pointed to is already free
    else if ((*((size_t *)ptr - 1) & 0x0) == 0)
    {
        return;
    }
    // Otherwise, provided block address is freed, size readjusted, payload rest to zero
    else
    {
        // temp variables to adjust fields of block struct
         

        size_t payload = size - (16);

        memset(temp, (size | 0), 8);
        temp = (temp + size) - (8);
        memset(temp, (size | 0), 8);
        temp = temp - payload;
        memset(temp, 0, (sizeof(size_t) * payload));        


        // change size to = header + footer + any optional padding + next pointer

        // figure out next block address by nextblock = currentblock.header + currentblock.size        
        
        // coalesce on every free() call. Need to implement. Just add to end of linked list.
    }


}



/*
 * realloc
 */
void* realloc(void* oldptr, size_t size)
{

    size_t sizeOld = *(HDRP(oldptr));
    size_t* new;
    // Unnecessary to check bit for a function that only accepts allocated blocks ---> maybe for edge case testing
    // size_t checAlloc = *(uint64_t *)(temp) & 0x1;
    
    if (oldptr == NULL)
    {    
        new = malloc(size);
    }
    else if (size == 0)
    {    
        free(oldptr);
    }
    else if (sizeOld == size)
    {
        return oldptr;
    }
    else
    {

        /* Must already be from m/c/realloc = check allocator bit
        malloc(size)
        memcpy(new, old, *pointer to size of old)
            or memcpy(new, old, size)
        free(old) and add to free list
        */
       new = malloc(size);
       mem_memcpy(new, oldptr, size);
       free(oldptr);

       return new;

    }
    return NULL;
}

/*
 * calloc
 * This function is not tested by mdriver, and has been implemented for you.
 */
void* calloc(size_t nmemb, size_t size)
{
    void* ptr;
    size *= nmemb;
    ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

/*
 * Returns whether the pointer is in the heap.
 * May be useful for debugging.
 */
static bool in_heap(const void* p)
{
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Returns whether the pointer is aligned.
 * May be useful for debugging.
 */
static bool aligned(const void* p)
{
    size_t ip = (size_t) p;
    return align(ip) == ip;
}

/*
 * mm_checkheap
 */
bool mm_checkheap(int lineno)
{
#ifdef DEBUG









#endif /* DEBUG */
    return true;
}
