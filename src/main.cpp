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
    char * a = (char *) bud_malloc(5);
    a[0] = 'a';
    a[1] = 'b';
    a[2] = 'c';
    a[3] = 'd';
    a[4] = '\0';
    printf("%s\n", a);
    return 0;
}
