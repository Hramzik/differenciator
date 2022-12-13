

#include "../headers/dfr.hpp"


int main (void) {

    //DFR START


    Dfr dfr = {};
    DFR_CTOR (&dfr);


    bool dumping = false;


    Dfr_settings settings = {};
    dfr_settings_ctor (&settings);
    get_dfr_settings  (&settings);


    Return_code return_code = SUCCESS;


    return_code = dfr_build_user_function (&dfr, &settings);
    printf ("reader        return code - %d\n", return_code);
    if (return_code) { dfr_sorry_message; return 0; }

    if (dumping) { FTREE_GRAPHDUMP (dfr.user_function_tree); }


    return_code = tex_generate_output    (&dfr, settings);
    printf ("tex generator return code - %d\n", return_code);
    if (return_code) { dfr_sorry_message; return 0; }


    if (dumping) { FTREE_GRAPHDUMP (dfr.derivative_trees_array [0]); }


    /*printf ("pdf generator return code - %d\n", tex_generate_pdf ());*/


    dfr_dtor (&dfr); printf ("goodbye!\n");


    return 0;
}