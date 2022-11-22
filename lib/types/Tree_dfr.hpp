#ifndef TYPE_TREE_DFR_TYPE_HPP_INCLUDED
#define TYPE_TREE_DFR_TYPE_HPP_INCLUDED
//--------------------------------------------------

#include "Stack.hpp"
//-------------------- SETTINGS --------------------
#define ON_TREE_ERROR_DUMPING
#define ON_TREE_AFTER_OPERATION_DUMPIN

#define tree_dump_bgclr       "9aabba"
#define tree_dump_nodeclr     "8bbfec"
#define tree_dump_leafclr     "88CDBE"
#define tree_dump_arrwclr     "0368c0"
#define tree_dump_freearrwclr "000000"

#define tree_default_save_file_name      "work/base.txt"
#define tree_dump_file_name              "work/tree_dump.txt"
#define tree_graph_dump_file_name        "work/graph_dump"
#define tree_graph_file_name             "graph"
#define tree_graph_describtion_file_name "work/graph_describtion"

const size_t GRAPH_WIDTH = 1460;

const bool POLITE_MODE = true;

const size_t MAX_GRAPH_DUMP_NUM = 100;
const size_t MAX_QUESTION_LEN   = 100;
const size_t MAX_ANSWER_LEN     = 100;
const size_t MAX_COMMAND_LEN    = 100;
const size_t DEFAULT_COUNTING_SYSTEM = 10;
//--------------------------------------------------


#define TREE_CTOR(x) _tree_ctor (x, #x + (#x[0] == '&'), __FILE__, __PRETTY_FUNCTION__, __LINE__)


#define FTREE_DUMP(x, ...)\
\
    if (!strcmp ("", #__VA_ARGS__)) {\
\
        _ftree_dump  (x, tree_dump_file_name, __FILE__, __PRETTY_FUNCTION__, __LINE__, "unknown");\
    }\
\
    else {\
\
        _ftree_dump  (x, tree_dump_file_name, __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);\
    }


#define FTREE_GRAPHDUMP(x, ...)\
\
    _tree_generate_graph_describtion (x);\
    _tree_generate_graph             ();\
\
\
    if (!strcmp ("", #__VA_ARGS__)) {\
\
        _ftree_graphdump (x, tree_graph_dump_file_name, __FILE__, __PRETTY_FUNCTION__, __LINE__, "");\
    }\
\
    else {\
\
        _ftree_graphdump (x, tree_graph_dump_file_name, __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);\
    }\
\
\
    _tree_show_graph_dump ()


#ifdef ON_TREE_ERROR_DUMPING

    #define ASSERT_TREE_OK(x) if (tree_damaged (x)) { FTREE_DUMP (x); LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define TREE_ERROR_DUMP(x) FTREE_DUMP(x)

#else

    #define ASSERT_TREE_OK(x) if (tree_damaged (x)) {                 LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    #define TREE_ERROR_DUMP(x)

#endif


#ifdef ON_TREE_AFTER_OPERATION_DUMPING
    #define TREE_AFTER_OPERATION_DUMPING(x) FTREE_DUMP (x)
#else
    #define TREE_AFTER_OPERATION_DUMPING(x)
#endif



typedef int Tree_state;
enum        Tree_state_flags  {

    TR_OK                            = 0, //TR - short for Tree_state
    TR_NULLPTR                       = 1,
    TR_NULLPTR_ROOT                  = 2,
    TR_INCORRECT_POISON_DISTRIBUTION = 4,
};


enum Atom_type {

    DAT_OPERATION = 0,
    DAT_CONST     = 1,
    DAT_VARIABLE  = 2,
};


typedef struct Tree_Node_structure Node;
struct         Tree_Node_structure  {

    Element   element;
    Atom_type atom_type;
    Node*     left_son;
    Node*     right_son;
};
const size_t NODE_SIZE = sizeof (Node);



typedef struct Tree_structure      Tree;
typedef struct Tree_info_structure Tree_info;
struct         Tree_info_structure  {

    const  char* name;
    Tree*        adress;
    const  char* birth_file;
    const  char* birth_func;
    int          birth_line;
};

struct  Tree_structure  {

    Node* root;

    Tree_info debug_info;
};

const size_t TREE_SIZE = sizeof (Tree);


typedef struct Tree_iterator_structure Tree_iterator;
struct         Tree_iterator_structure  {

    Node*  current;
    size_t index;
    size_t depth;
    Stack* node_stack;

    const char* mode;
};

Return_code _tree_ctor (Tree* tree, const char* name, const char* file, const char* func, int line);
Return_code  tree_dtor (Tree* tree);
Return_code _node_dtor (Node* node);

Return_code  tree_push_left  (Tree* tree, Node* node, Element_value new_element_value, Atom_type atom_type);
Return_code  tree_push_right (Tree* tree, Node* node, Element_value new_element_value, Atom_type atom_type);

size_t tree_depth (Tree* tree);

Tree_state  tree_damaged             (Tree* tree);
Tree_state _tree_poison_distribution (Tree* tree);
Tree_state _node_poison_distribution (Node* node);

void         _ftree_dump                      (Tree* tree, const char* file_name, const char* file, const char* func, int line, const char* file_mode = "unknown");
Return_code  _fdump_nodes                     (FILE* dump_file, Tree* tree, const char* mode = "pre");
void         _fprint_tabs                     (FILE* file, size_t num);
void         _ftree_graphdump                 (Tree* tree, const char* file_name, const char* file, const char* func, int line, const char* additional_text = "");
void         _tree_show_graph_dump            (void);
void         _tree_generate_graph_describtion (Tree* tree);
void         _tree_generate_nodes_describtion (Tree* tree, FILE* file);
const char*  _operation_code_to_str           (Operation_code operation_code);
void         _tree_generate_graph             (void);

Return_code tree_iterator_inc            (Tree_iterator* tree_iterator);
Return_code _tree_iterator_inc_in_order  (Tree_iterator* tree_iterator);
Return_code _tree_iterator_inc_pre_order (Tree_iterator* tree_iterator);
Return_code tree_iterator_ctor           (Tree_iterator* tree_iterator, Tree* tree, const char* inc_mode = "pre");
Return_code tree_iterator_dtor           (Tree_iterator* tree_iterator);

Return_code _tree_iterator_ctor_fill_stack_in_order  (Tree_iterator* tree_iterator, Tree* tree);
Return_code _tree_iterator_ctor_fill_stack_pre_order (Tree_iterator* tree_iterator, Tree* tree);

bool _ispositive (const char* answer);
bool _isnegative (const char* answer);
bool _isleaf     (Node* node);

void _speak (const char* format, ...);
//--------------------------------------------------
#endif