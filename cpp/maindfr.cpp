

#include "../headers/dfr.hpp"


int main (void) {

    //DFR START


    Dfr dfr = {};
    DFR_CTOR (&dfr);
    printf ("read return code - %d\n", dfr_read_user_function (&dfr));


    tex_generate_output (&dfr, "x", 2);


    return 0;
}