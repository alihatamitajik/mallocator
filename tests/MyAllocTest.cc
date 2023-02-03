#include <gtest/gtest.h>
#include <string.h>
#include <limits.h>
#include "myalloc.h"


TEST(BuddyMallocTest, ShouldAllocate)
{
    char *str = (char *) bud_malloc(5, 0);
    ASSERT_NO_THROW(strcpy(str, "abcd"));
}

TEST(BuddyMallocTest, ShouldAllocateWhenFreeIsAvailable)
{
    void *a = bud_malloc(10, 0);
    bud_free(a);
    void *b = bud_malloc(10, 0);
    ASSERT_EQ(a, b);
}

TEST(BuddyMallocTest, ShouldSplitWhenBigger)
{
    void* a = bud_malloc(30, 0);
    bud_free(a);
    void *b = bud_malloc(5, 0), *c = bud_malloc(5, 0);
    ASSERT_EQ(a, b);
    ASSERT_EQ(c, (void *)((long)a + 64));
}

TEST(BuddyMallocTest, ShouldNullWhenCant)
{
    // void* a;
    // // Allocate 100GB which is not available
    // for (size_t i = 0; i < 1000; i++)
    // {
    //     a = bud_malloc(100000000, 0);
    // }
}

TEST(BuddyMallocTest, ShouldFill)
{
    int* a = (int *)bud_malloc(sizeof(int), 0);
    ASSERT_EQ(0, *a);
}

TEST(BuddyFreeTest, ShouldFreeMultipleTime)
{
    // for (size_t i = 0; i < 1000; i++)
    // {
    //     void *a = bud_malloc(100000000, 0);
    //     bud_free(a);
    // }
}

TEST(BuddyFreeTest, ShouldCoalesceWhenFree)
{
    void *a = bud_malloc(5, 0), *b = bud_malloc(5, 0);
    bud_free(a);
    bud_free(b);
    void *c = bud_malloc(20, 0);
    ASSERT_EQ(c, a);
}


TEST(BuddyReallocTest, ShouldNullIfCant)
{
    // TODO: sbrk does not return -1 and program gets killed :)
}

TEST(BuddyReallocTest, ShouldFreeWhenZeroSize)
{
    void *a = bud_malloc(5, 0);
    bud_realloc(a, 0, 0);
    void *b = bud_malloc(5, 0);
    ASSERT_EQ(a,b);
}


TEST(BuddyReallocTest, ShouldMallocWhenNullPtr)
{
    char *a = (char *)bud_realloc(NULL, 5, 0);
    ASSERT_NO_THROW(strcpy(a, "abcd"));
}

TEST(BuddyReallocTest, ShouldSplitIfSmaller)
{
    void *a = bud_malloc(20, 0);
    bud_realloc(a, 5, 0);
    // should be in the space of the previous
    void *b = bud_malloc(5, 0);
    ASSERT_EQ((void *)((long) a + 64), b);
}

TEST(BuddyReallocTest, ShouldLimitBoundaries)
{
    bud_set_minimum(10);
    bud_set_maximum(15);
    void *a = bud_malloc(5,0);
    void *b = bud_malloc(20,0);
    bud_set_minimum(4);
    void *c = bud_malloc(5, 0);
    bud_set_maximum(21);
    void *d = bud_malloc(20, 0);
    bud_set_maximum(-1);
    void *e = bud_malloc(200, 0);
    ASSERT_EQ(a, (void *) NULL);
    ASSERT_EQ(b, (void *) NULL); 
    ASSERT_FALSE(c == NULL);
    ASSERT_FALSE(d == NULL);
    ASSERT_FALSE(e == NULL);
}
