#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "mm.h"
#include "memlib.h"

#define ALIGNMENT 16

static bool in_heap(const void* p)
{
    return p <= mm_heap_hi() && p >= mm_heap_lo();
}

// Pointer to epilogue
size_t *epilogue_ptr;

//Pointer to start of 15 free lists
size_t *free_list_24;
size_t *free_list_40;
size_t *free_list_56;
size_t *free_list_72;
size_t *free_list_88;
size_t *free_list_104;
size_t *free_list_120;
size_t *free_list_136;
size_t *free_list_152;
size_t *free_list_168;
size_t *free_list_184;
size_t *free_list_200;
size_t *free_list_216;
size_t *free_list_232;
size_t *free_list_big;

// Helper function to add to free list, assume given address of header of new free block
void add_to_free_list(size_t *new_block){

    size_t **free_list_ptr;

    // find what free list block should be added to
    if (*new_block <= 24)
    {
        free_list_ptr = &free_list_24;
    }

    else if (*new_block <= 40)
    {
        free_list_ptr = &free_list_40;
    }

    else if (*new_block <= 56)
    {
        free_list_ptr = &free_list_56;
    }

    else if (*new_block <= 72)
    {
        free_list_ptr = &free_list_72;
    }

    else if (*new_block <= 88)
    {
        free_list_ptr = &free_list_88;
    }

    else if (*new_block <= 104)
    {
        free_list_ptr = &free_list_104;
    }

    else if (*new_block <= 120)
    {
        free_list_ptr = &free_list_120;
    }

    else if (*new_block <= 136)
    {
        free_list_ptr = &free_list_136;
    }

    else if (*new_block <= 152)
    {
        free_list_ptr = &free_list_152;
    }

    else if (*new_block <= 168)
    {
        free_list_ptr = &free_list_168;
    }

    else if (*new_block <= 184)
    {
        free_list_ptr = &free_list_184;
    }

    else if (*new_block <= 200)
    {
        free_list_ptr = &free_list_200;
    }

    else if (*new_block <= 216)
    {
        free_list_ptr = &free_list_216;
    }

    else if (*new_block <= 232)
    {

        free_list_ptr = &free_list_232;
    }

    else if (*new_block > 232)
    {
        free_list_ptr = &free_list_big;
    }

    // List is empty, becomes head
    if ((*free_list_ptr) == NULL){

        // Set head of free list to be this new block
        (*free_list_ptr) = new_block;

        // Move to prev metadata, set to 0 because no prev
        new_block += 1;
        size_t prev_data = 0;
        memcpy(new_block, &prev_data, 8);

        // Move to next metadata, set to 0 because no next, can reuse prev data val
        new_block += 1;
        memcpy(new_block, &prev_data, 8);
    }

    // Free list is not empty
    else{

        // Make copy of head of free list
        size_t *copy_of_head = (*free_list_ptr);

        // Set new block to be new head
        (*free_list_ptr) = new_block;

        // Move to prev metadata of current head and set to new block
        copy_of_head += 1;
        memcpy(copy_of_head, &new_block, 8);

        // Move current head back to header
        copy_of_head -= 1;

        // Set prev of new head, will be 0 because no prev
        new_block += 1;
        size_t prev_data = 0;
        memcpy(new_block, &prev_data, 8);

        // Set next of new head to be current head
        new_block += 1;
        memcpy(new_block, &copy_of_head, 8);

    }

    return;
}

// Helper function to remove a block from free list
void remove_from_free_list(size_t *block_to_remove){

    size_t **free_list_ptr;

    // find what free list block is in
    if (*block_to_remove <= 24)
    {
        free_list_ptr = &free_list_24;
    }

    else if (*block_to_remove <= 40)
    {
        free_list_ptr = &free_list_40;
    }

    else if (*block_to_remove <= 56)
    {
        free_list_ptr = &free_list_56;
    }

    else if (*block_to_remove <= 72)
    {
        free_list_ptr = &free_list_72;
    }

    else if (*block_to_remove <= 88)
    {
        free_list_ptr = &free_list_88;
    }

    else if (*block_to_remove <= 104)
    {
        free_list_ptr = &free_list_104;
    }

    else if (*block_to_remove <= 120)
    {
        free_list_ptr = &free_list_120;
    }

    else if (*block_to_remove <= 136)
    {
        free_list_ptr = &free_list_136;
    }

    else if (*block_to_remove <= 152)
    {
        free_list_ptr = &free_list_152;
    }

    else if (*block_to_remove <= 168)
    {
        free_list_ptr = &free_list_168;
    }

    else if (*block_to_remove <= 184)
    {
        free_list_ptr = &free_list_184;
    }

    else if (*block_to_remove <= 200)
    {
        free_list_ptr = &free_list_200;
    }

    else if (*block_to_remove <= 216)
    {
        free_list_ptr = &free_list_216;
    }

    else if (*block_to_remove <= 232)
    {

        free_list_ptr = &free_list_232;
    }

    else if (*block_to_remove > 232)
    {
        free_list_ptr = &free_list_big;
    }
    
    // Make copy of block to remove
    size_t *copy_of_block_to_remove = block_to_remove;


    // Move to prev section and Get prev metadata
    copy_of_block_to_remove += 1;
    size_t *prev_metadata = (copy_of_block_to_remove);
    

    // Move to next sectiona and get next metadata
    copy_of_block_to_remove += 1;
    size_t *next_metadata = (copy_of_block_to_remove);

    
    // Case 1: Only one block in list
    if (((*prev_metadata) == 0) && ((*next_metadata) == 0)){

        
        // Just have to set free list pointer to null
        (*free_list_ptr) = NULL;


        return;
    }

    // Case 2: Block is the head and there are multiple blocks in list
    if (block_to_remove == (*free_list_ptr)){

        
        // make copy of head of free list
        size_t *copy_of_head = (*free_list_ptr);

        // Move to next section of current head, extract the address it will be new head
        copy_of_head += 2;
        size_t *new_head = (size_t*)*(copy_of_head);

        // Set new head to be the next of the current head
        (*free_list_ptr) = new_head;

        // Set prev of new head to be 0 (None)
        new_head += 1;
        size_t new_prev_metadata = 0;
        memcpy(new_head, &new_prev_metadata, 8);

        // get rid of prev and next metatdata for block we are removing
        size_t val = 0;
        memcpy(prev_metadata, &val, 8);
        memcpy(next_metadata, &val, 8);

        return;

    }

    // Case 3: block to remove is tail and there are mutliple blocks in list
    if (((*prev_metadata) != 0) && ((*next_metadata) == 0)){

        // change next section in previous block
        size_t *left_next = (size_t*)((size_t*)(*prev_metadata) + 2);
        size_t new_next_metadata = 0;
        memcpy(left_next, &new_next_metadata, 8);

        // get rid of prev and next metatdata for block we are removing
        size_t val = 0;
        memcpy(prev_metadata, &val, 8);
        memcpy(next_metadata, &val, 8);

        return;

    }

    // Case 4: block to remove is in between blocks
    if (((*prev_metadata) != 0) && ((*next_metadata) != 0)){

        // need to change the next of left block to be right block
        size_t **left_next = (size_t**)((size_t*)(*prev_metadata) + 2);
        memcpy(left_next, next_metadata, 8);


        // need to change prev of right block to be left block
        size_t **right_prev = (size_t**)((size_t*)(*next_metadata) + 1);
        memcpy(right_prev, prev_metadata, 8);

        // get rid of prev and next metatdata for block we are removing
        size_t val = 0;
        memcpy(prev_metadata, &val, 8);
        memcpy(next_metadata, &val, 8);

        return;
    }
}


// rounds up to the nearest multiple of ALIGNMENT
static size_t align(size_t x)
{
    return ALIGNMENT * ((x+ALIGNMENT-1)/ALIGNMENT);
}

/*
 * mm_init: returns false on error, true on success.
 */
bool mm_init(void)
{

    // make an empty free lists, will clear the free lists if new init
    free_list_24 = NULL;
    free_list_40 = NULL;
    free_list_56 = NULL;
    free_list_72 = NULL;
    free_list_88 = NULL;
    free_list_104 = NULL;
    free_list_120 = NULL;
    free_list_136 = NULL;
    free_list_152 = NULL;
    free_list_168 = NULL;
    free_list_184 = NULL;
    free_list_200 = NULL;
    free_list_216 = NULL;
    free_list_232 = NULL;
    free_list_big = NULL;

    // Initialize Heap with 32 bytes
    void *heap_ptr = mm_sbrk(32);

    // check if heap expansion worked, if not then return false
    if (heap_ptr == NULL){
        return false;
    }

    // Move past the padding, set prologue header to size 0 allocated
    heap_ptr = (char*)heap_ptr + 8;
    size_t prologue_data = 1;
    memcpy(heap_ptr, &prologue_data, sizeof(size_t));

    // Move to prologue footer, set prologue footer to be size 0 allocated
    heap_ptr = (char*)heap_ptr + 8;
    memcpy(heap_ptr, &prologue_data, sizeof(size_t));

    // Move to epilogue and Set the epilogue metadata (metadata will be 1 since size 0 and allocated)
    heap_ptr = (char*)heap_ptr + 8;
    size_t epilogue_data = 1;
    memcpy(heap_ptr, &epilogue_data, sizeof(size_t));
    epilogue_ptr = heap_ptr;

    return true;
}

/*
 * malloc
 */
void* malloc(size_t size)
{

    // Return pointer
    void* return_ptr;

    // 62 ones then a 0 then a 1
    size_t prev_bitmask_val = 18446744073709551613ULL;

    // Used for free list ptr
    size_t **free_list_looper = NULL;

    // Align the requested size to 16 bytes
    // payload is max(align(size+8), 32) - 8
    size_t temp = align(size + 8);
    if (temp < 32){
        temp = 32;
    }
    size_t aligned_requested_size = temp - 8;

    // get appropriate free list for request
    if ((aligned_requested_size <= 24) && (free_list_24 != NULL))
    {
        free_list_looper = &free_list_24;
    }

    else if((aligned_requested_size <= 40) && (free_list_40 != NULL))
    {
        free_list_looper = &free_list_40;
    }

    else if ((aligned_requested_size <= 56) && (free_list_56 != NULL))
    {
        free_list_looper = &free_list_56;
    }

    else if ((aligned_requested_size <= 72) && (free_list_72 != NULL))
    {
        free_list_looper = &free_list_72;
    }

    else if ((aligned_requested_size <= 88) && (free_list_88 != NULL))
    {
        free_list_looper = &free_list_88;
    }

    else if ((aligned_requested_size <= 104) && (free_list_104 != NULL))
    {
        free_list_looper = &free_list_104;
    }

    else if ((aligned_requested_size <= 120) && (free_list_120 != NULL))
    {
        free_list_looper = &free_list_120;
    }

    else if ((aligned_requested_size <= 136) && (free_list_136 != NULL))
    {
        free_list_looper = &free_list_136;
    }

    else if ((aligned_requested_size <= 152) && (free_list_152 != NULL))
    {
        free_list_looper = &free_list_152;
    }

    else if ((aligned_requested_size <= 168) && (free_list_168 != NULL))
    {
        free_list_looper = &free_list_168;
    }

    else if ((aligned_requested_size <= 184) && (free_list_184 != NULL))
    {
        free_list_looper = &free_list_184;
    }

    else if ((aligned_requested_size <= 200) && (free_list_200 != NULL))
    {
        free_list_looper = &free_list_200;
    }

    else if ((aligned_requested_size <= 216) && (free_list_216 != NULL))
    {
        free_list_looper = &free_list_216;
    }

    else if ((aligned_requested_size <= 232) && (free_list_232 != NULL))
    {

        free_list_looper = &free_list_232;
    }

    else if (free_list_big != NULL)
    {
        free_list_looper = &free_list_big;
    }

    // Free list is not empty, Loop through free list 
    if (free_list_looper != NULL){

        // Make copy of this header for removing from free list
        size_t *copy_of_header = *free_list_looper;

        while ((copy_of_header != 0)){

            // Extract size from header
            size_t mem_size = (*copy_of_header);

            // check if requested data can fit, if it does then insert there, return pointer
            if (aligned_requested_size <= mem_size)
            {

                // Not enough space to split block, 8 for header, 24 minimum for mem_size
                if ((aligned_requested_size + 32 > mem_size))
                {
                    
                    // Remove block from free list
                    remove_from_free_list(copy_of_header);

                    // Change header to be allocated
                    (*copy_of_header) = (*copy_of_header) | 1;


                    // Move past header to mem section, store this pointer to return
                    copy_of_header += 1;
                    return_ptr = (void *)(copy_of_header);

                    // go to next header and get rid of prev bit
                    copy_of_header += (mem_size / 8);
                    (*copy_of_header) = (*copy_of_header) & prev_bitmask_val;


                    // Return the pointer
                    return return_ptr;
                }

                // Can split the block
                else
                {


                    // Going to allocate this block so remove from free list
                    remove_from_free_list(copy_of_header);

                    // Add alloc bit to size for new header, and store
                    size_t new_header = aligned_requested_size | 1;
                    memcpy(copy_of_header, &new_header, 8);

                    // Move to mem section, end of header, and store this pointer
                    copy_of_header += 1;
                    return_ptr = (void *)(copy_of_header);

                    // Move to header of new split block, create it and store, used 8 for header so subtract
                    copy_of_header += (aligned_requested_size / 8);
                    size_t split_block_mem_size = mem_size - aligned_requested_size - 8;
                    // Alloc bit of 1, then send to free to reuse free logic
                    size_t split_block_header = split_block_mem_size | 1;
                    memcpy(copy_of_header, &split_block_header, 8);

                    // free new split block and pass ptr to mem section
                    free(copy_of_header + 1);
                    
                    // Return the pointer to user
                    return return_ptr;
                }
            }

            // otherwise move to next section of current free block to loop
            else
            {
                copy_of_header = (size_t*)*((size_t*)(copy_of_header) + 2);
                
            }
        }
    }

    // Either free list is empty or couldnt find suitable block

    // Try to Expand the heap for header, requested size
    void *sbrk_request = mm_sbrk(8 + aligned_requested_size);

    // Check if sbrk returns null, if it does need to return null for malloc
    if (sbrk_request == NULL)
    {
        return NULL;
    }

    // Shift epilogue to end of heap, accounting for new header, mem section
    size_t *new_epilogue = epilogue_ptr + 1 + (aligned_requested_size / 8);
    memcpy(new_epilogue, epilogue_ptr, sizeof(size_t));

    // Current epilogue spot will be new block spot
    size_t *new_block_spot = epilogue_ptr;

    // update epilogue ptr
    epilogue_ptr = new_epilogue;

    // Set header for new block, add alloc bit to header
    size_t header_data = aligned_requested_size | 1;
    memcpy(new_block_spot, &header_data, sizeof(size_t));

    // Move past header to user malloc section, save this pointer, make sure to turn ptr to void
    new_block_spot += 1;
    return_ptr = (void *)new_block_spot;

    return return_ptr;
}

/*
 * free
 */
void free(void* ptr)
{

    // Used for bitmasking (2^64 - 2) (63 ones then 1 zero)
    size_t bitmask_val = 18446744073709551614ULL;
    

    // if null ptr do nothing
    if (ptr != NULL) {

        // see if ptr is in heap
        if (in_heap(ptr) == false){
            return;
        }

        size_t left_mem_size;
        size_t right_mem_size;
        size_t *right_block_header;

        // 62 ones then a 0 then a 1
        size_t prev_bitmask_val = 18446744073709551613ULL;

        // Corresponding header
        size_t *header_ptr = (size_t*)((char*)ptr - 8);

        // Make temporary copy of heap pointer to use for looping
        size_t *header_copy = header_ptr;

        // Extract prev free bit from header
        size_t prev_free = (*header_copy) & 2;

        // clear prev free bit from header
        (*header_copy) = (*header_copy) & prev_bitmask_val;

        // Extract size from header
        size_t mem_size = (*header_copy) & bitmask_val;

        // preset to left is allocated then change based on prev bit
        int left_alloc = 1;


        // not zero means previous block is free
        if (prev_free != 0){

            // left block is free
            left_alloc = 0;

            // can go to left footer and extract size
            header_copy -= 1;
            left_mem_size = (*header_copy);

            // reset header_copy
            header_copy += 1;
        }

    
        // go to right block header
        header_copy += 1 + (mem_size / 8);

        // preset right block to alloc then change if its free
        int right_alloc = 1;

        // make sure its not epilogue on right
        if (*header_copy != 1){

            // extract right block alloc bit
            right_alloc = (*header_copy) & 1;


            // Right block is free
            if (right_alloc == 0){

                // store right block info
                right_block_header = header_copy;
                right_mem_size = (*header_copy);
            }

            // Right block is allocated, add prev free bit
            else{
                (*header_copy) = (*header_copy) | 2;
            }
            
        }


        // reset heap_copy to original header
        header_copy = header_ptr;

        // have cases to see if can coalesc
        
        // case 1: left and right are free
        if ((left_alloc == 0) && (right_alloc == 0))
        {

            // left mem size + middle mem size + right mem size  + header + header
            size_t total_mem_size = left_mem_size + mem_size + right_mem_size + 8 + 8;

            // go to left header (new header)
            header_copy -= ((left_mem_size / 8) + 1);
            // delete left block from free list, becauase that size is outdated now
            // also need to delete right block from free list because that cannot be used anymore
            remove_from_free_list(right_block_header);
            remove_from_free_list(header_copy);

            // new combined block
            size_t new_header = total_mem_size;
            memcpy(header_copy, &new_header, 8);
    
            // add updated size free block to list
            add_to_free_list(header_copy);

            // go to start of new mem section
            header_copy += 1;

            // go to right footer (new footer), contents same as header
            header_copy += ((total_mem_size / 8) - 1);
            memcpy(header_copy, &new_header, 8);

            return;
        }

        // case 2: just left is free
        if ((left_alloc == 0))
        {

    
            // left mem + header + middle mem size
            size_t total_mem_size = left_mem_size + 8 + mem_size;

            // go to left header (new header)
            header_copy -= ((left_mem_size / 8) + 1);
            // delete this free block from free list
            remove_from_free_list(header_copy);
            size_t new_header = total_mem_size;
            memcpy(header_copy, &new_header, 8);
            // add updated size free block to list
            add_to_free_list(header_copy);

            // go to start of new mem section
            header_copy += 1;

            // go to middle footer, (new footer), identical contents to header
            header_copy += ((total_mem_size / 8) - 1);
            memcpy(header_copy, &new_header, 8);

            return;
        }

        // case 3: just right is free
        if ((right_alloc == 0))
        {
            
            // delete right block from free list
            remove_from_free_list(right_block_header);

            // total size is mem size + header + right mem size
            size_t total_mem_size = mem_size + 8 + right_mem_size;

            // already at the new header, set it
            size_t new_header = total_mem_size;
            memcpy(header_copy, &new_header, 8);

            add_to_free_list(header_copy);

            // move to mem section
            header_copy += 1;
            

            // go to right footer, (new footer)
            header_copy += ((total_mem_size / 8) - 1);
            memcpy(header_copy, &new_header, 8);

            return;
        }

        // case 4: neither left nor right is free so just free the block, code below

        // Change header to be free alloc bit
        size_t new_header = mem_size;
        memcpy(header_copy, &new_header, 8);

        // Move to footer, change it to be free alloc bit, identical to header
        header_copy += 1 + (mem_size / 8) - 1;
        memcpy(header_copy, &new_header, 8);

        // add to free list
        add_to_free_list(header_ptr);

        return;
    }

    return;
}

/*
 * realloc
 */
void* realloc(void* oldptr, size_t size)
{
    void *return_ptr;

    // Used for bitmasking (2^64 - 2) (63 ones then 1 zero)
    size_t bitmask_val = 18446744073709551614ULL;

    // Null ptr case
    if (oldptr == NULL){
        return_ptr = malloc(size);
        return return_ptr;
    }

    // Size 0 case
    if (size == 0){
        free(oldptr);
        return NULL;
    }

    // Realloc Case

    // Corresponding header
    size_t *header_ptr = (size_t*)((char*)oldptr - 8);

    // Make copy of header
    size_t *header_copy = header_ptr;

    // Get aligned size
    size_t aligned_size = align(size);

    // 62 ones then a 0 then a 1
    size_t prev_bitmask_val = 18446744073709551613ULL;

    
    // Extract size from header, get rid of alloc bit
    size_t mem_size = (*header_copy) & bitmask_val;


    // get rid of prev free bit to get size
    mem_size = mem_size & prev_bitmask_val;

    // Same Size
    if (aligned_size == mem_size)
    {
        return oldptr;
    }

    // Shrinking Case
    if (aligned_size < mem_size)
    {


        // use block splitting code, 8 for header, 24 minimum mem_size
        if (aligned_size + 32 < mem_size)
        {
            // similar align logic like in malloc
            size_t temp = align(size + 8);
            if (temp < 32){
            temp = 32;
            }
            size_t aligned_size = temp - 8;

            // Add alloc bit to size for new header, and store,
            size_t new_header = aligned_size | 1;
            memcpy(header_copy, &new_header, 8);

            // Move to mem section, end of header, and store this pointer
            header_copy += 1;
            return_ptr = (void *)header_copy;

            // Move to header of new split block, create it and store, used 8 for header so subtract
            header_copy += (aligned_size / 8);
            size_t split_block_mem_size = mem_size - aligned_size - 8;
            // Alloc bit of 1, will alloc then send to free to reuse free logic
            size_t split_block_header = split_block_mem_size | 1;
            memcpy(header_copy, &split_block_header, 8);

            free(header_copy + 1);

            // Return the pointer to user
            return return_ptr;
        }

        // Cannot split the block
        else
        {

            // Give them back same pointer
            return oldptr;
        }
    }

    // Expanding Case
    if (aligned_size > mem_size)
    {

        // Approach will be to free the current block and allocate new one of the bigger size and copy contents

        // make new block
        void *new_block_ptr = malloc(aligned_size);


        if (new_block_ptr == NULL)
        {
            return NULL;
        }

        // Copy over contents from old block
        memcpy(new_block_ptr, oldptr, mem_size);


        // Free old block
        free(oldptr);

        // Return new ptr
        return new_block_ptr;
    }

    return NULL;
}




