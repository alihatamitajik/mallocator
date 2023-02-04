#include "myalloc.h"
#include <stdio.h>


int main(int argc, char const *argv[])
{
    set_algorithm("buddy");

    int *data, *data2, *d1, *d2;

    data = (int *)my_malloc(100000 * sizeof(int), 0);
    data2 = (int *)my_malloc(100000 * sizeof(int), 0);

    show_stats();

    for (int i = 0; i < 100000; i++) {
        data[i] = i;
        data2[i] = i;
    }
    for (int i = 0; i < 100000; i++) {
        printf("%x: %d\n", &data[i], data[i]);
    }
    for (int i = 0; i < 100000; i++) {
        printf("%x: %d\n", &data2[i], data2[i]);
    }
    my_free(data2);
    my_realloc(data, 150000, 0);
    my_free(data);

    d1 = (int *)my_malloc(1000, 0);
    d2 = (int *)my_malloc(1000, 0);
    my_free(d1);
    my_free(d2);

    show_stats();

    return 0;
}
