// note: these files are provided by Ali Hatami, and are written by them.


/*
 * firstfit.c
 *
 * proper documentation is added for each function (mostly in the header file).
 */

#include "firstfit.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h> 

#define MIN(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#define MAX(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

/** Initial Min limit (no limit) */
long ff_min_limit = 0;

/** initial Max limit (no limit) */
long ff_max_limit = -1;

/* this struct is created to manage block pointers */
struct b_list {
    s_block_ptr first;
    s_block_ptr last;
} b_list = {NULL, NULL};

int are_limits_set = 0;
size_t minAllocation = (size_t) NULL;
size_t maxAllocation = (size_t) NULL;

void define_min_max_allocation(size_t min, size_t max){
    are_limits_set = 1;
    minAllocation = min;
    maxAllocation = max;
}

/**
 * @brief moves header of the b to the new_start and add diff to its size
 * 
 * @param b block to be moved. It should be FREE. 
 * @param new_start new location of the block.
 */
void move_is_free_block_back (s_block_ptr b, void *new_start) {
    size_t diff = (void *) b - new_start;
    memmove(new_start, b, BLOCK_SIZE);
    b = (s_block_ptr) new_start;
    b->prev->next = b;
    b->size = b->size + diff;
    if (b->next != NULL) {
        b->next->prev = b;
    }
    b->ptr = &b->data;
}

/**
 * @brief Splits the block with size s and make another is_free part out of the
 *        other part.
 * 
 * If size of the block b is less than s nothing will be done but if the size
 * is bigger than s, there is some scenarios:
 * 
 *      1. there is another is_free part after this block, then the remaning will 
 *      be fused together and make a larger block. In this case the header will be
 *      moved to the is_free part and the size will be added to the current size
 *      of the next block.
 *
 *      2. size of the remaining part is bigger than the SIZE_BLOCK and
 *      metadata part can fit into it, so a new is_free part will be created and
 *      prev and next of the three blocks will be updated.
 * 
 *      3.1. remaining is less than BLOCK_SIZE but it is at the end of the 
 *      allocated memories so we will set break to the end of the segment that
 *      is needed and if the rest is needed will be allocated later.
 *
 *      3.2. size of the remaining part is not enough to create a new block so
 *      we assume that part is no man land and we won't tell user that!
 * 
 * @param b the block that will be splitted - should be a valid block
 * @param s size of the block AFTER reduction
 */
void split_block (s_block_ptr b, size_t s) {
    if (b->size <= s) 
        return;
    
    void *end_of_b = b->ptr + s;
    if (b->next != NULL && b->next->is_free) {
        move_is_free_block_back (b->next, end_of_b);
        b->size = s;
    } else if (b->next == NULL) {
        brk(end_of_b);
        b->size = s;
    } else if (b->size - s >= BLOCK_SIZE) {
        s_block_ptr new_block = (s_block_ptr) end_of_b;
        s_block_ptr next = b->next;
        b->next = new_block;
        new_block->prev = b;
        new_block->next = next;
        next->prev = new_block;
        new_block->size = b->size - s - BLOCK_SIZE;
        new_block->ptr = &new_block->data;
        new_block->is_free = 1;
        b->size = s;
        b_list.last = new_block;
    }
}


/**
 * @brief fuse two prior and late blocks
 * 
 * It will remove metadata of the late block and update metadata of the prior
 * block. Both prior and late must be FREE and valid blocks. 
 * 
 * @param prior the block that will expanded
 * @param late  the block which will be fused to the other, it must be after
 *              the prior block
 */
void ff_fuse (s_block_ptr prior, s_block_ptr late) {
    prior->next = late->next;
    if (late->next != NULL) {
        late->next->prev = prior;
    } else {
        b_list.last = prior;
    }
    prior->size = prior->size + late->size + BLOCK_SIZE;
}


/**
 * @brief Try to fuse is_free blocks before and after of b together
 * 
 * it will fuse is_free adjacent blocks together. It only checks prev and next
 * blocks because if it be used after each ff_is_free then there only can't be
 * other sequence of is_free blocks (they where fused together when one of them
 * was is_freed)!
 * 
 * @param b the block to perform possible fusions on
 * @return pointer to the new b (the block that b was fused to) 
 */
s_block_ptr fusion (s_block_ptr b) {
    if (!b->is_free) {
        return b;
    }

    if (b->prev != NULL && b->prev->is_free) {
        s_block_ptr prev = b->prev;
        ff_fuse (prev, b);
        b = prev;
    }

    if (b->next != NULL && b->next->is_free) {
        ff_fuse (b, b->next);
    }

    if (b->next == NULL) {
        b_list.last = b;
    }

    return b;
}

/**
 * @brief Get the block object corresponding to ptr
 * 
 * For sanity this function will iterate over all allocated blocks and retrive
 * the ptr of that. because ptr may be invalid and looking for BLOCK_SIZE bytes
 * ahead may cause conflicts and security issues.
 * 
 * @param p start of allocated memory
 * @return s_block_ptr 
 */
s_block_ptr ff_get_block (void *p) {
    if (p == NULL) {
        return NULL;
    }

    s_block_ptr sb = b_list.first;
    while (sb) {
        if (sb->ptr == p) {
            return sb;
        }
        sb = sb->next;
    }

    return NULL;
}

/**
 * @brief allocate memory to the end of the heap
 * 
 * This function will be used when there is not enough space in the allocated
 * memory. First it will check if there is is_free space at the end. if there were
 * one, it will allocate remaining amount of memory and add it to that block. 
 * If there wasn't a is_free block at the last a new block will be constructed. at
 * any time that allocation can't be done it will return NULL.
 * 
 * @param last the last element of the block lists.
 * @param s size to be allocated
 * @return NULL on failure and a pointer to the newly allocated(expanded) block
 */
s_block_ptr ff_extend_heap (s_block_ptr last , size_t s) {
    void *mem;

    if (last != NULL && last->is_free) {
        mem = sbrk(s - last->size);
        if (mem == (void *) -1) {
            return NULL;
        }

        last->size = s;
        return last;
    }
    
    mem = sbrk(BLOCK_SIZE + s);
    if (mem == (void *) -1) {
        return NULL;
    }

    s_block_ptr header = (s_block_ptr) mem;
    header->ptr = &header->data;
    header->is_free = 1;
    header->next = NULL;
    header->prev = last;
    header->size = s;
    if (last == NULL) {
        b_list.first = header;
    } else {
        last->next = header;
    }
    b_list.last = header;
    return header;
}


/**
 * @brief finds a block with first fit or allocate a new one
 * 
 * iterate over allocated blocks to find first-fit block 
 * - first block that was found will be splitted for the new data
 * - if none was found heap will be extended
 * 
 * @param size 
 * @return s_block_ptr 
 */
s_block_ptr get_first_fit (size_t size) {
    s_block_ptr sb = b_list.first;
    while (sb) {
        if (sb->is_free && sb->size >= size) {
            /* memory should be splitted */
            if (sb->size > size) {
                split_block(sb, size);
            }
            return sb;
        }
        sb = sb->next;
    }

    /* if reached here no enough space was found  we should extend the heap */
    sb = ff_extend_heap (b_list.last, size);

    return sb;
}

void* ff_malloc(size_t size, int fill)
{  
    /* returns NULL if the size is zero or the size does not match the min and max constraints */
    if (size < ff_min_limit || (ff_max_limit != -1 && size > ff_max_limit))
    {
        return NULL;
    }

    if (size <= 0) {
        return NULL;
    }

    s_block_ptr sb = get_first_fit (size);
    if (sb == NULL) {
        return NULL;
    } else {
        sb->is_free = 0;
        memset(sb->ptr, fill, size);
        return sb->ptr;
    }
}


void* ff_realloc(void* ptr, size_t size, int fill)
{
    if (size == 0) {
        ff_free (ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return ff_malloc(size, fill);
    }

    s_block_ptr sb = ff_get_block (ptr);
    if (sb == NULL || sb->is_free) {
        return NULL;
    }

    if (sb->size == size) 
    {
        return sb->ptr;
    }

    if (sb->size > size && size >= ff_min_limit)
    {
        split_block(sb, size);
        return sb->ptr;
    }
    

    void *new_mem = ff_malloc(size, fill);
    if (new_mem == NULL) {
        return NULL;
    }

    memcpy(new_mem, sb->ptr, MIN(size, sb->size));
    /* freeing sb */
    sb->is_free = 1;
    fusion(sb);
    return new_mem;
}


void ff_free(void* ptr)
{
    s_block_ptr sb = ff_get_block (ptr);

    if (sb == NULL) {
        /* if the pointer in not to a valid block */
        return;
    } else {
        /* else it should set FREE state to 1 and fuse if available */
        sb->is_free = 1;
        fusion(sb);
    }
}


int ff_set_minimum(int min)
{
    if (ff_max_limit == -1 || min <= ff_max_limit)
    {
        ff_min_limit = MAX(0, min);
    }
    return ff_min_limit;
}


int ff_set_maximum(int max)
{
    if (max == -1)
    {
        ff_max_limit = -1;
    } else if (max > ff_min_limit)
    {
        ff_max_limit = MAX(1, max);
    }
    return ff_max_limit;
}

void ff_show_stats(){
    unsigned allocated = ff_show_stats_by_type(0);
    unsigned not_allocated = ff_show_stats_by_type(1);
    printf("total allocated: %d\ntotal free: %d\n", allocated, not_allocated);
    void* sbrk_pointer = sbrk(0);
    printf("sbrk pointer and allocated + free difference: %u\n", sbrk_pointer - (allocated + not_allocated));
}

int ff_show_stats_by_type(int is_free){

    s_block_ptr first = (s_block_ptr) b_list.first;
    s_block_ptr temp = (s_block_ptr) b_list.first;
    
    unsigned total_size = 0;

    if(is_free){
        printf("showing free blocks:\n");
    }else{
        printf("showing allocated blocks:\n");
    }

    while (1){
        if (b_list.first == NULL){
            break;
        }
        if (temp->is_free == is_free){
            printf("start_address: %10u, end_address: %10u, size: %10u\n", temp->ptr, temp->ptr + temp->size, temp->size);
            total_size += temp->size;
        }
        if (temp->next == NULL){
            break;
        }
        temp = (s_block_ptr)(temp->next);
    }
    return total_size;
}