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

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

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
void fuse (s_block_ptr prior, s_block_ptr late) {
    prior->next = late->next;
    if (late->next != NULL) {
        late->next->prev = prior;
    } else {
        b_list.last = prior;
    }
    prior->size = prior->size + late->size + BLOCK_SIZE;
}


s_block_ptr fusion (s_block_ptr b) {
    if (!b->is_free) {
        return b;
    }

    if (b->prev != NULL && b->prev->is_free) {
        s_block_ptr prev = b->prev;
        fuse (prev, b);
        b = prev;
    }

    if (b->next != NULL && b->next->is_free) {
        fuse (b, b->next);
    }

    if (b->next == NULL) {
        b_list.last = b;
    }

    return b;
}


s_block_ptr get_block (void *p) {
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


s_block_ptr extend_heap (s_block_ptr last , size_t s) {
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
    sb = extend_heap (b_list.last, size);

    return sb;
}

void* mm_malloc(size_t size)
{  
    /* returns NULL if the size is zero or the size does not match the min and max constraints */

    if (size <= 0) {
        return NULL;
    }

    s_block_ptr sb = get_first_fit (size);
    if (sb == NULL) {
        return NULL;
    } else {
        sb->is_free = 0;
        memset(sb->ptr, 0, size);
        return sb->ptr;
    }
}


void* mm_realloc(void* ptr, size_t size)
{
    if (size == 0) {
        mm_free (ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return mm_malloc(size);
    }

    s_block_ptr sb = get_block (ptr);
    if (sb == NULL || sb->is_free) {
        return NULL;
    }

    void *new_mem = mm_malloc(size);
    if (new_mem == NULL) {
        return NULL;
    }

    memcpy(new_mem, sb->ptr, min(size, sb->size));
    /* freeing sb */
    sb->is_free = 1;
    fusion(sb);
    return new_mem;
}


void mm_free(void* ptr)
{
    s_block_ptr sb = get_block (ptr);

    if (sb == NULL) {
        /* if the pointer in not to a valid block */
        return;
    } else {
        /* else it should set FREE state to 1 and fuse if available */
        sb->is_free = 1;
        fusion(sb);
    }
}

