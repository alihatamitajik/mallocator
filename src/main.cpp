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
    void * a = bud_malloc(5, 0), *b = bud_malloc(20, 0);
    bud_free(b);
    a = bud_realloc(a, 20, 0);
    printf("%d\n", a == b);
}
