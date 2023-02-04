#include <gtest/gtest.h>
#include <string.h>
#include <limits.h>
#include "myalloc.h"
#include <sys/resource.h>


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
    struct rlimit lim;

    lim.rlim_cur = 0x800000;
    lim.rlim_max = 0x800000;

    // set limit to 8MBs
    ASSERT_EQ(0, setrlimit(RLIMIT_DATA, &lim));
    // we should make sure rounding up does not exceed the limit
    void *a = bud_malloc(2000, 0);
    void *b = bud_malloc(0x400000, 0);
    ASSERT_EQ(b, (void *) NULL);
    ASSERT_FALSE(a == NULL); 
}

TEST(BuddyMallocTest, ShouldFill)
{
    int* a = (int *)bud_malloc(sizeof(int), 0);
    ASSERT_EQ(0, *a);
}

TEST(BuddyFreeTest, ShouldFreeMultipleTime)
{
    for (size_t i = 0; i < 1000; i++)
    {
        void *a = bud_malloc(100000000, 0);
        bud_free(a);
    }
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
    struct rlimit lim;

    lim.rlim_cur = 0x800000;
    lim.rlim_max = 0x800000;

    // set limit to 8MBs
    ASSERT_EQ(0, setrlimit(RLIMIT_DATA, &lim));
    // we should make sure rounding up does not exceed the limit
    void *a = bud_malloc(2000, 0);
    void *b = bud_realloc(a, 0x400000, 0);
    ASSERT_EQ(b, (void *) NULL);
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

TEST(FirstfitMallocTest, ShouldAllocate)
{
    char *str = (char *) ff_malloc(5, 0);
    ASSERT_NO_THROW(strcpy(str, "abcd"));
}

TEST(FirstfitMallocTest, ShouldAllocateWhenFreeIsAvailable)
{
    void *a = ff_malloc(10, 0);
    ff_free(a);
    void *b = ff_malloc(10, 0);
    ASSERT_EQ(a, b);
}

TEST(FirstfitMallocTest, ShouldSplitWhenBigger)
{
    void* a = ff_malloc(100, 0);
    ff_free(a);
    void *b = ff_malloc(5, 0), *c = ff_malloc(5, 0);
    ASSERT_EQ(a, b);
    ASSERT_EQ(c, (void *)((long)a + 45));
}

TEST(FirstfitMallocTest, ShouldNullWhenCant)
{
    struct rlimit lim;

    lim.rlim_cur = 0x800000;
    lim.rlim_max = 0x800000;

    // set limit to 8MBs
    ASSERT_EQ(0, setrlimit(RLIMIT_DATA, &lim));
    // we should make sure rounding up does not exceed the limit
    void *a = ff_malloc(2000, 0);
    void *b = ff_malloc(0x800000, 0);
    ASSERT_EQ(b, (void *) NULL);
    ASSERT_FALSE(a == NULL); 
}

TEST(FirstfitMallocTest, ShouldFill)
{
    int* a = (int *)ff_malloc(sizeof(int), 0);
    ASSERT_EQ(0, *a);
}

TEST(FirstfitFreeTest, ShouldFreeMultipleTime)
{
    for (size_t i = 0; i < 1000; i++)
    {
        void *a = ff_malloc(100000000, 0);
        ff_free(a);
    }
}

TEST(FirstfitFreeTest, ShouldCoalesceWhenFree)
{
    void *a = ff_malloc(5, 0), *b = ff_malloc(5, 0);
    ff_free(a);
    ff_free(b);
    void *c = ff_malloc(20, 0);
    ASSERT_EQ(c, a);
}


TEST(FirstfitReallocTest, ShouldNullIfCant)
{
    struct rlimit lim;

    lim.rlim_cur = 0x800000;
    lim.rlim_max = 0x800000;

    // set limit to 8MBs
    ASSERT_EQ(0, setrlimit(RLIMIT_DATA, &lim));
    // we should make sure rounding up does not exceed the limit
    void *a = ff_malloc(2000, 0);
    void *b = ff_realloc(a, 0x800000, 0);
    ASSERT_EQ(b, (void *) NULL);
}

TEST(FirstfitReallocTest, ShouldFreeWhenZeroSize)
{
    void *a = ff_malloc(5, 0);
    ff_realloc(a, 0, 0);
    void *b = ff_malloc(5, 0);
    ASSERT_EQ(a,b);
}


TEST(FirstfitReallocTest, ShouldMallocWhenNullPtr)
{
    char *a = (char *)ff_realloc(NULL, 5, 0);
    ASSERT_NO_THROW(strcpy(a, "abcd"));
}

TEST(FirstfitReallocTest, ShouldSplitIfSmaller)
{
    void *a = ff_malloc(100, 0);
    ff_realloc(a, 5, 0);
    // should be in the space of the previous
    void *b = ff_malloc(5, 0);
    ASSERT_EQ((void *)((long) a + 45), b);
}

TEST(FirstfitReallocTest, ShouldLimitBoundaries)
{
    ff_set_minimum(10);
    ff_set_maximum(15);
    void *a = ff_malloc(5,0);
    void *b = ff_malloc(20,0);
    ff_set_minimum(4);
    void *c = ff_malloc(5, 0);
    ff_set_maximum(21);
    void *d = ff_malloc(20, 0);
    ff_set_maximum(-1);
    void *e = ff_malloc(200, 0);
    ASSERT_EQ(a, (void *) NULL);
    ASSERT_EQ(b, (void *) NULL); 
    ASSERT_FALSE(c == NULL);
    ASSERT_FALSE(d == NULL);
    ASSERT_FALSE(e == NULL);
}
