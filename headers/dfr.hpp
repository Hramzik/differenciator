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

#define dfr_default_input_file_name "work/test.txt"
#define dfr_default_dump_file_name  "work/dfr_dump.txt"

const size_t MAX_VARIABLE_LEN = 100;
//--------------------------------------------------


#define DFR_CTOR(x, ...) _dfr_ctor (x, #x + (#x[0] == '&'), __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define FDFR_DUMP(x, file, mode, ...)\
\
    if (!strcmp ("", #__VA_ARGS__)) {\
\
        _fdfr_dump  (x, file, __FILE__, __PRETTY_FUNCTION__, __LINE__, mode, "");\
    }\
\
    else {\
\
        _fdfr_dump  (x, file, __FILE__, __PRETTY_FUNCTION__, __LINE__, mode, ##__VA_ARGS__);\
    }

#define DFR_DUMP(x, ...) FDFR_DUMP (x, dfr_default_dump_file_name, "unknown", ##__VA_ARGS__)


#define FDFR_GRAPHDUMP(x, ...)\
\
    if (!strcmp ("", #__VA_ARGS__)) {\
\
        _fdfr_graphdump (x, tree_graph_dump_file_name, __FILE__, __PRETTY_FUNCTION__, __LINE__, "");\
    }\
\
    else {\
\
        _fdfr_graphdump (x, tree_graph_dump_file_name, __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);\
    }



#ifdef ON_DFR_ERROR_DUMPING

    #define ASSERT_DFR_OK(x) if (tree_damaged (x)) { DFR_DUMP (x); LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define DFR_ERROR_DUMP(x) DFR_DUMP(x)

#else

    #define ASSERT_DFR_OK(x) if (tree_damaged (x)) {                 LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define DFR_ERROR_DUMP(x)

#endif


#ifdef ON_DFR_AFTER_OPERATION_DUMPING
    #define DFR_AFTER_OPERATION_DUMPING(x) DFR_DUMP (x)
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

    Tree* user_function_tree;


    Dfr_info debug_info;
};

const size_t DFR_SIZE = sizeof (Dfr);


Return_code _dfr_ctor (Dfr* dfr, const char* name, const char* file, const char* func, int line);


void _fdfr_dump      (Dfr* dfr, const char* file_name, const char* file, const char* function, int line, const char* file_mode, const char* additional_text = "");
void _fdfr_graphdump (Dfr* dfr, const char* file_name, const char* file, const char* function, int line, const char* additional_text = "");

size_t get_operation_priority (Operation_code operation_code);

Return_code read_number   (const char** string_ptr, Node** node_ptr);
Return_code read_variable (const char** string_ptr, Node** node_ptr);
Return_code read_primary  (const char** string_ptr, Node** node_ptr);
Return_code read_product  (const char** string_ptr, Node** node_ptr);
Return_code read_sum      (const char** string_ptr, Node** node_ptr);
Return_code read_general  (const char*  string,     Tree*  tree);

bool is_variable_start (char simbol);
bool is_variable_mid   (char simbol);

Return_code read_user_function (Dfr* dfr, const char* file_name = dfr_default_input_file_name);

Node* create_node (Atom_type atom_type, ...);
/*

Return_code  dfr_save (Tree* tree, const char* file_name = tree_default_save_file_name);
Return_code  dfr_read (Tree* tree, const char* file_name = tree_default_save_file_name);
Return_code _dfr_read (char* buffer, size_t* current, Tree* tree, Node* node);

*/


//--------------------------------------------------
#endif