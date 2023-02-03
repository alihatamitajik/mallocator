// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

/**
 * @brief Splits the block into half
 * 
 * hard-minimum size is 64, because we should be able to keep the header meta.
 * So, hard-minimum for requested size should be 16 bytes
 * 
 * 
 * 
 * @param b the block that will be splitted - should be a valid block
 */
int split (bud_block_ptr b)
{

}


/**
 * @brief Try to coalesce is_free blocks as much as possible
 * 
 * This function use an iterative behavior (i.e. it coalesce blocks if it was
 * successful it tries to coalesce blocks another time until it cant anymore).
 * 
 * This way we make sure that all possible combination of coalesce happened,
 * although this method is compute intensive and better implementations was
 * possible.
 * 
 * @return int 1 if a coalesce occurred and 0 if not. 
 */
int coalesce()
{
    return NULL;
}

/**
 * @brief Get the block object corresponding to ptr
 * 
 * 
 * @param p start of allocated memory
 * @return s_block_ptr 
 */
s_block_ptr get_block (void *p)
{

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
s_block_ptr extend_heap (s_block_ptr last , size_t s)
{

}


