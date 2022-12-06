#ifndef ELEMENT_atom_HPP_INCLUDED
#define ELEMENT_atom_HPP_INCLUDED









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



enum Operation_code {

    DOC_UNKNOWN = 0, //DOC - short for differenciator_operation_code 
    DOC_ADD,
    DOC_SUB,
    DOC_DIV,
    DOC_MULT,
    DOC_POW,
    DOC_LN,
    DOC_SIN,
    DOC_COS,
    DOC_TG,
    DOC_CTG,
};

union Atom {

    Operation_code val_operation_code;
    double         val_double;
    char*          var_str;
};


typedef Atom Element_value;


Element_value const _poisoned_Element_value = { DOC_UNKNOWN };


typedef struct Element_structure Element;
struct         Element_structure  {

    Element_value value;
    bool          poisoned;
};


const size_t ELEMENT_SIZE = sizeof (Element);









#endif