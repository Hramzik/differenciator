

#include "../headers/calc.hpp"


int main (void) {

    //CALC START


    char entered [100] = "";


    while (entered[0] != 'q') {

        scanf ("%s", entered);
        printf ("%d\n\n", get_general (entered));
    }


    return 0;
}