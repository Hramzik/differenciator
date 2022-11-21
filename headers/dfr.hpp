#ifndef DIFFERENCIATOR_HPP_INCLUDED
#define DIFFERENCIATOR_HPP_INCLUDED


//--------------------------------------------------
#include <sys\stat.h>
#include <locale.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>


#include "../lib/types/Return_code.hpp"

#include "../lib/types/Elements/use_Element_atom.hpp"
#include "../lib/types/Elements/Element_atom.hpp"


#include "../lib/types/Tree_dfr.hpp"


//-------------------- SETTINGS --------------------
#define ON_DFR_ERROR_DUMPING
#define ON_DFR_AFTER_OPERATION_DUMPIN


//--------------------------------------------------


#define DFR_CTOR(x, ...)\
\
    if (!strcmp ("", #__VA_ARGS__)) {\
\
        _tree_ctor (x, #x + (#x[0] == '&'), __FILE__, __PRETTY_FUNCTION__, __LINE__, "UNITIALIZED_STRING");\
    }\
\
    else {\
\
        _tree_ctor (x, #x + (#x[0] == '&'), __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);\
    }


#define FDFR_DUMP(x) _fdfr_dump  (x, tree_dump_file_name, __FILE__, __PRETTY_FUNCTION__, __LINE__)


#define FDFR_GRAPHDUMP(x, ...)\
\
    if (!strcmp ("", #__VA_ARGS__)) {\
\
        FTREE_GRAPHDUMP (x.code, "");\
    }\
\
    else {\
\
        FTREE_GRAPHDUMP (x.code, ##__VA_ARGS__);\
    }



#ifdef ON_DFR_ERROR_DUMPING

    #define ASSERT_DFR_OK(x) if (tree_damaged (x)) { FDFR_DUMP (x); LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define DFR_ERROR_DUMP(x) FDFR_DUMP(x)

#else

    #define ASSERT_DFR_OK(x) if (tree_damaged (x)) {                 LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define DFR_ERROR_DUMP(x)

#endif


#ifdef ON_DFR_AFTER_OPERATION_DUMPING
    #define DFR_AFTER_OPERATION_DUMPING(x) FDFR_DUMP (x)
#else
    #define DFR_AFTER_OPERATION_DUMPING(x)
#endif



typedef struct Dfr_structure      Dfr;
typedef struct Dfr_info_structure Dfr_info;

struct         Dfr_info_structure  {

    const  char* name;
    Dfr*         adress;
    const  char* birth_file;
    const  char* birth_func;
    int          birth_line;
};

struct Dfr_structure {

    Tree* code;


    Dfr_info debug_info;
};

const size_t DFR_SIZE = sizeof (Dfr);


Return_code _dfr_ctor (Dfr* dfr, const char* name, const char* file, const char* func, int line);


void _fdfr_dump  (Dfr* dfr, const char* file_name, const char* file, const char* function, int line);

size_t get_operation_priority (Operation_code operation_code);
/*

Return_code  dfr_save (Tree* tree, const char* file_name = tree_default_save_file_name);
Return_code  dfr_read (Tree* tree, const char* file_name = tree_default_save_file_name);
Return_code _dfr_read (char* buffer, size_t* current, Tree* tree, Node* node);

*/


//--------------------------------------------------
#endif