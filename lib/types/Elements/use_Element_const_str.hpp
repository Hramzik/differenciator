

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

