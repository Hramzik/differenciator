

#include "../headers/dfr.hpp"


int main (void) {

    //DFR START


    Dfr dfr = {};
    DFR_CTOR (&dfr);


    printf ("reader        return code - %d\n", dfr_read_user_function (&dfr));
    printf ("tex generator return code - %d\n", tex_generate_output    (&dfr, "x", 6));


    return 0;
}