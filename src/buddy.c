// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "buddy.h"
#include <string.h>

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

bud_meta head = NULL;

/** Initial Min limit (no limit) */
long min_limit = 0;

/** initial Max limit (no limit) */
long max_limit = -1;

size_t sum_allocated = 0;


/**
 * @brief return the smallest power of two value greater than x ([1], p 48)
 * 
 * Other methods was possible for this work like using A32 builtin bit scan 
 * revers (bsr) instruction or using mathematical approach with logarithms and
 * ceiling.
 * 
 * [1] Baxter, Michael. "hacker's delight." Linux Journal 2003.108 (2003).
 *
 */
static __attribute__((noinline)) unsigned next_pow2(unsigned x)
{
	x -= 1;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	
	return x + 1;
}


/**
 * @brief Splits the block into half
 * 
 * When we are splitting, it will take the block into two headers and adjust
 * the depth and the Rightness of it.
 * 
 * NOTE: we know that the size is a power of two. We also know that split will
 * not happen for sizes less than 128 bytes (because the minimum request will
 * be 64).
 * 
 * 
 * @param b the block that will be splitted - should be a valid block
 */
void split (bud_meta b)
{
    size_t half_size = b->size / 2;
    if (b->size <= 64)
        return;

    // find header
    bud_meta next_half = (bud_meta)((void *) b + half_size);

    // adjust sizes
    next_half->size = b->size = half_size;

    // adjust depth and rightness
    next_half->depth = (b->depth += 1); 
    next_half->rightness = next_half->rightness << 1 + 1;
    b->rightness <<= 1;
    
    // adjust the links
    next_half->prev = b;
    next_half->next = b->next;
    b->next = next_half;

    // adjust the ptr
    next_half->ptr = &next_half->data;

    // adjust freeness
    next_half->is_free = 1;
}


/**
 * @brief combines two block together
 * 
 * It should remove the right header from the list, update the depth and 
 * rightness of it. 
 * 
 * @param left 
 * @param right 
 */
void fuse(bud_meta left, bud_meta right)
{
    left->next = right->next;
    left->size = left->size * 2;
    left->depth -= 1;
    left->rightness >>= 1;
}


/**
 * @brief Try to coalesce is_free blocks as much as possible
 * 
 * This function will check if a fusion is possible:
 *  - if it's left (right) child it will check if right (left) child is free and
 *    if it was, a merge will happen between these two. if succeeded, it will
 *    call the function recursively to merge with the other one.
 * 
 * @param bm newly freed block.
 */
void coalesce(bud_meta bm)
{
    if (bm->rightness & 1 && bm->is_free) // if it's right child 
    {
        bud_meta left = bm->prev;
        if (left != NULL && left->is_free && left->depth == bm->depth)
        {
            fuse(left, bm);
            coalesce(left);
        }
    } else {
        bud_meta right = bm->next;
        if (right != NULL && right->is_free && right->depth == bm->depth)
        {
            fuse(bm, right);
            coalesce(bm);
        }
    }
}

/**
 * @brief Get the block object corresponding to ptr
 * 
 * iterates over headers and return the block if it has matching prt
 * 
 * @param p start of allocated memory
 * @return s_block_ptr 
 */
bud_meta get_block (void *ptr)
{
    bud_meta block = head;
    while (block)
    {
        if (block->ptr == ptr) 
            return block;
        block = block->next;
    }

    // if nothing found
    return NULL;
}

/**
 * @brief double the size of the heap allocated by now
 * 
 * This functions double the memory we had allocated previously (sum_allocated)
 * and update the depth of the depth of previous data.
 * 
 * NOTE: each time we extend the heap, depth of all blocks should be increased
 * by one.
 * 
 * NOTE: since we are adding the newly block to the right, we don't need to
 * update the rightness of the metadata.
 * 
 * @param last the last element of the block lists.
 * @param s size to be allocated
 * @return NULL on failure and a pointer to the newly allocated(expanded) block
 */
bud_meta extend_heap ()
{
    void* mem;
    mem = (void *) sbrk(sum_allocated);
    if (mem == (void *) -1)
    {
        return NULL;
    }

    bud_meta header = (bud_meta) mem;

    // increment the depth of previous
    bud_meta prev, block = head;
    while (block)
    {
        block->depth += 1;
        prev = block;
        block = block->next;
    }

    // define the new block header
    header->size = sum_allocated;
    header->is_free = 1;
    header->rightness = 1;
    header->depth = 1;
    header->ptr = &header->data;

    // adjust the links
    header->next = NULL;
    header->prev = prev;
    prev->next = header;

    sum_allocated <<= 1;

    return header;
}

/**
 * @brief Initializes the heap by size given to it
 * 
 * @param s size of allocation
 * @return bud_meta 
 */
bud_meta init_heap(size_t size)
{
    void* mem;
    mem = (void *) sbrk(size);
    
    if (mem == (void*) -1)
    {
        return NULL; // couldn't allocate
    } 
    
    bud_meta header = (bud_meta) mem;
    header->size = size;
    header->is_free = 1;
    header->next = NULL;
    header->prev = NULL;
    header->depth = 0;
    header->rightness = 0;
    header->ptr = &header->data;
    head = header;

    sum_allocated = size;

    return header;
}


/**
 * @brief Get the smallest fit of the data or find smallest to split
 * 
 * @param size 
 * @return bud_meta 
 */
bud_meta get_best_fit(size_t size)
{
    bud_meta block = head;
    bud_meta best = NULL;
    while (block)
    {
        if (block->is_free)
        {
            if (block->size == size)
            {
                return block;
            }
            else if (block->size > size 
                     && (best == NULL || best->size > block->size))
            {
                best = block;
            }
        }
        
        block = block->next;
    }

    return best;
}

bud_meta shrink_to_size(bud_meta bm, size_t size)
{
    if (bm->size < size) {
        return NULL;
    } else {
        while (bm->size > size)
        {
            split(bm);
        }
        return bm;
    }
}

/**
 * @brief Get the block pointer
 * 
 * for more information see the bud_malloc documentation
 * 
 * @param size size to get
 * @return bud_meta NULL if couldn't else a pointer to header of *free* data
 */
bud_meta alloc_block(size_t size)
{
    
    if (head == NULL)
    { // if no allocation before this extend the heap
        return init_heap(size);
    } 
    else 
    { /* find the best fit block and if not found extend the heap
         extension is done until it */
        bud_meta best_fit = get_best_fit(size);
        if (best_fit != NULL)
        {
            return shrink_to_size(best_fit, size);
        } else { // no fitted block found, we should extend the heap
            bud_meta last_extended;
            do
            {
                last_extended = extend_heap();
                if (last_extended == NULL)
                    return NULL;
            } while (sum_allocated/2 < size);
            // shrink the new allocated heap to the size we wanted
            return shrink_to_size(last_extended, size);
        }
    }
}

void* bud_malloc(size_t size, int fill)
{
    // if it violates the boundaries
    if (size < min_limit || (max_limit != -1 && size > max_limit))
    {
        return NULL;
    }
    size_t request = next_pow2(BLOCK_SIZE + size);
    bud_meta bbp = alloc_block(request);
    if (bbp == NULL) 
    {
        return NULL;
    } else {
        bbp->is_free = 0;
        memset(bbp->ptr, fill, request - BLOCK_SIZE);
        return bbp->ptr;
    }
}


void free_block(bud_meta bm)
{
    bm->is_free = 1;
    coalesce(bm);
}


void* bud_realloc(void* ptr, size_t size, int fill)
{
    if(size <= 0) {
        bud_free(ptr);
        return NULL;
    }
    
    if (ptr == NULL) {
        return bud_malloc(size, fill);
    }

    bud_meta bm = get_block(ptr);
    if (bm == NULL || bm->is_free) {
        return NULL;
    }

    size_t request = next_pow2(BLOCK_SIZE + size);

    if (bm->size == request) {
        return bm->ptr;
    }

    // We need a bigger space
    void* new_mem = bud_malloc(size, fill);
    if (new_mem == NULL)
    {
        return NULL;
    }
    memcpy(new_mem, bm->ptr, MIN(size, bm->size));
    free_block(bm);
    return new_mem;
}


void bud_free(void* ptr)
{
    bud_meta block  = get_block(ptr);
    if (block != NULL)
    {
        free_block(block);
    }
}

void bud_show_stats(){
    unsigned allocated = bud_show_stats_by_type(0);
    unsigned not_allocated = bud_show_stats_by_type(1);
    printf("total allocated: %d\ntotal free: %d\n", allocated, not_allocated);
    void* sbrk_pointer = sbrk(0);
    printf("sbrk pointer and allocated + free difference: %u\n", sbrk_pointer - (allocated + not_allocated));
}

int bud_show_stats_by_type(int is_free){

    bud_meta first = head;
    if (head == NULL){
        return 0;
    }
    bud_meta temp = head->next;
    unsigned total_size = 0;

    if(is_free){
        printf("showing free blocks:\n");
    }else{
        printf("showing allocated blocks:\n");
    }

    while (1){
        if (first == NULL){
            break;
        }
        if (temp->is_free == is_free){
            printf("start_address: %10u, end_address: %10u, size: %10u\n", temp->ptr, temp->ptr + temp->size, temp->size);
            total_size += temp->size;
        }
        if (temp->next == NULL){
            break;
        }
        temp = temp->next;
    }
    return total_size;
}

int bud_set_minimum(int min)
{
    if (min <= max_limit)
    {
        min_limit = MAX(0, min);
    }
    return min_limit;
}


int bud_set_maximum(int max)
{
    if (max == -1)
    {
        max_limit = -1;
    } else if (max > min_limit)
    {
        max_limit = MIN(1, max);
    }
    return max_limit;
}