

#include "../headers/dfr.hpp"


int main (void) {

    //DFR START


    Dfr dfr = {};
    DFR_CTOR (&dfr);


    bool INTERACTIVE_MODE = false;

    char  variable [MAX_VARIABLE_LEN] = "x";
    double taylor_point               = 1;
    size_t depth                      = 6;
    double tangent_point              = 1;
    int precision                     = 1;


    if (INTERACTIVE_MODE) {

        printf ("\n");
        printf ("please, enter variable name:\n\n>> "); scanf ("%s",   variable);      printf ("\n");
        printf ("please, enter taylor point:\n\n>> ");  scanf ("%lf", &taylor_point);  printf ("\n");
        printf ("please, enter taylor depth:\n\n>> ");  scanf ("%zd", &depth);         printf ("\n");
        printf ("please, enter tangent point:\n\n>> "); scanf ("%lf", &tangent_point); printf ("\n");
        printf ("please, enter precision:\n\n>> ");     scanf ("%d",  &precision);     printf ("\n");
    }


    Return_code return_code = SUCCESS;



    return_code = dfr_read_user_function (&dfr);
    printf ("reader        return code - %d\n", return_code); 
    if (return_code) { dfr_sorry_message; return 0; }

    //FTREE_GRAPHDUMP (dfr.user_function_tree);


    return_code = tex_generate_output    (&dfr, variable, taylor_point, depth, tangent_point, precision);
    printf ("tex generator return code - %d\n", return_code);
    if (return_code) { dfr_sorry_message; return 0; }


    /*printf ("pdf generator return code - %d\n", tex_generate_pdf ());*/


    dfr_dtor (&dfr); printf ("goodbye!\n");


    return 0;
}