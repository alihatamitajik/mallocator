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
    printf("%p, %p, %p, %p\n", 
    bud_malloc(5), 
    bud_malloc(5),
    bud_malloc(5),
    bud_malloc(5));
}
