# Mallocator

Sharif University of Technology
Operating Systems Course
Final Project

Mallocator is a custom memory management library that uses First-Fit and Buddy allocation algorithms. The library is powered by `sbrk` systemcall. This library uses strategy pattern and `myalloc.h` provides a wrapper around `firstfit.h` and `buddy.h`. 

User can set the allocation algorithm (using `set_algorithm`) once and only before using any of the `mm_*` functions (If it's not specified, first fit is the default choice).
