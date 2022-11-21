#ifndef ELEMENT_CONST_STR_HPP_INCLUDED
#define ELEMENT_CONST_STR_HPP_INCLUDED









#undef  Element_value
#define Element_value Element_const_str_value

#undef  Element
#define Element Element_const_str

#undef  _poisoned_Element_value
#define _poisoned_Element_value _poisoned_Element_const_str_value

#undef  Element_structure
#define Element_structure Element_const_str_structure

#undef  ELEMENT_SIZE
#define ELEMENT_SIZE ELEMENT_const_str_SIZE

#undef  _ELEMENT_PRINT_FORMAT
#define _ELEMENT_PRINT_FORMAT "%s"



typedef const char* Element_value;

Element_value const _poisoned_Element_value = nullptr;


typedef struct Element_structure Element;
struct         Element_structure  {

    Element_value value;
    bool          poisoned;
};


const size_t ELEMENT_SIZE = sizeof (Element);









#endif