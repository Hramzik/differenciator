

#include "../headers/calc.hpp"


int  get_number  (const char** string_ptr) {

    int ans = 0;

    const char* old_string = *string_ptr;

    while (**string_ptr >= '0' && **string_ptr <= '9') {

        ans = (ans * 10) + (**string_ptr - '0');
        *string_ptr += 1;
    }

    assert (*string_ptr > old_string);


    return ans;
}


int  get_primary  (const char** string_ptr) {

    int ans = 0;

    if (**string_ptr == '(') {

        *string_ptr += 1;
        ans = get_sum (string_ptr);
        *string_ptr += 1;
    }

    else { ans = get_number (string_ptr); }

    return ans;
}


int  get_product  (const char** string_ptr) {

    int ans = get_primary (string_ptr);

    while (**string_ptr == '*' || **string_ptr == '/') {

        char op = **string_ptr;
        *string_ptr += 1;
        int val2 = get_primary (string_ptr);

        if (op == '*') { ans *= val2; }
        else           { ans /= val2; }
    }


    return ans;
}


int  get_sum  (const char** string_ptr) {

    int ans = get_product (string_ptr);

    while (**string_ptr == '+' || **string_ptr == '-') {

        char op = **string_ptr;
        *string_ptr += 1;
        int val2 = get_product (string_ptr);

        if (op == '+') { ans += val2; }
        else           { ans -= val2; }
    }


    return ans;
}


int  get_general  (const char* string) {

    int ans = get_sum (&string);


    assert (*string == '\0');


    return ans;
}

