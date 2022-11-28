
#include "stdio.h"

#define test(...) test_num##__VA_OPT__(_with_arguments) (__VA_ARGS__)

#define test_num(...) printf ("no args given\n");
#define test_num_with_arguments(...) printf ("got some args!");


int main (void) {


    test ();


    return 0;
}

