

#include "../headers/dfr.hpp"


int main (void) {

    //DFR START


    Dfr dfr = {};
    DFR_CTOR (&dfr);
    printf ("return code - %d\n", read_user_function (&dfr));

    FDFR_GRAPHDUMP (&dfr);


    return 0;
}