

#include "../headers/dfr.hpp"


int main (void) {

    //DFR START


    Dfr dfr = {};
    DFR_CTOR (&dfr);
    printf ("read      return code - %d\n", dfr_read_user_function        (&dfr));
    printf ("write1    return code - %d\n", dfr_write_user_function       (&dfr));
    printf ("calculate return code - %d\n", dfr_calculate_derivative_tree (&dfr, "y"));
    printf ("write2    return code - %d\n", dfr_write_derivative          (&dfr));


    FDFR_GRAPHDUMP (&dfr);


    return 0;
}