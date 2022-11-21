#ifndef ELEMENT_PTR_HPP_INCLUDED
#define ELEMENT_PTR_HPP_INCLUDED









#undef  Element_value
#define Element_value Element_ptr_value

#undef  Element
#define Element Element_ptr

#undef  _poisoned_Element_value
#define _poisoned_Element_value _poisoned_Element_ptr_value

#undef  Element_structure
#define Element_structure Element_ptr_structure

#undef  ELEMENT_SIZE
#define ELEMENT_SIZE ELEMENT_ptr_SIZE

#undef  _ELEMENT_PRINT_FORMAT
#define _ELEMENT_PRINT_FORMAT "%p"



typedef void* Element_value;

Element_value const _poisoned_Element_value = nullptr;


typedef struct Element_structure Element;
struct         Element_structure  {

    Element_value value;
    bool          poisoned;
};


const size_t ELEMENT_SIZE = sizeof (Element);









#endif