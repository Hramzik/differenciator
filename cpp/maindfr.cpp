

#include "../headers/dfr.hpp"


int main (void) {

    //DFR START


    Dfr dfr = {};
    DFR_CTOR (&dfr);
    printf ("read       return code - %d\n", dfr_read_user_function        (&dfr));
    printf ("write0     return code - %d\n", dfr_write_user_function       (&dfr));

    printf ("calculate1 return code - %d\n", dfr_calculate_derivative_tree (&dfr, "x"));
    printf ("write1     return code - %d\n", dfr_write_derivative          (&dfr));

    printf ("calculate2 return code - %d\n", dfr_calculate_taylor (&dfr, "x", 7));
    printf ("write2     return code - %d\n", dfr_write_taylor     (&dfr, "x", 7));


    tex_generate_output (&dfr);


    return 0;
}