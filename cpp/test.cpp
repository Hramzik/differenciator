
#include "stdio.h"
#include "../lib/types/Elements/Element_atom.hpp"
#include "../lib/types/Elements/use_Element_atom.hpp"


int main (void) {


    Atom a = { .val_double = 66666 };


    printf ("%d\n%lf\n%p", a.val_operation_code, a.val_double, a.var_str);


    return 0;
}

