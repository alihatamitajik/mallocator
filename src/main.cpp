/**
 * Copyright (c) 2023 Ali Hatami
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "../include/buddy.h"
#include <stdio.h>


int main(int argc, char const *argv[])
{
    void *a = bud_malloc(20, 0);
    bud_realloc(a, 5, 0);
    // should be in the space of the previous
    void *b = bud_malloc(5, 0);
}
