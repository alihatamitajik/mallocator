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
    void * a = bud_malloc(5), *b = bud_malloc(5);
    printf("%p, %p\n", a, b);
    bud_free(a);
    bud_free(b);
    printf("%p\n", bud_malloc(20));
}
