

#undef  Element_value
#define Element_value Element_atom_value

#undef  Element
#define Element Element_atom

#undef  _poisoned_Element_value
#define _poisoned_Element_value _poisoned_Element_atom_value

#undef  Element_structure
#define Element_structure Element_atom_structure

#undef  ELEMENT_SIZE
#define ELEMENT_SIZE ELEMENT_atom_SIZE

#undef  _ELEMENT_PRINT_FORMAT
#define _ELEMENT_PRINT_FORMAT "%p"

