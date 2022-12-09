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
#define USER_FUNCTION_NAME "f"


#define ON_DFR_ERROR_DUMPING
#define ON_DFR_AFTER_OPERATION_DUMPIN

#define dfr_default_input_file_name        "work/test.txt"
#define dfr_default_output_file_name       "work/answer.txt"
#define dfr_default_tex_file_name          "work/texoutput.tex"
#define dfr_default_dump_file_name         "work/dfr_dump.html"
#define dfr_gnu_plot_describtion_file_name "work/function_graph.gnu"
#define dfr_gnu_plot_file_name             "work/function_graph.png"

const size_t MAX_VARIABLE_LEN = 100;
const size_t MAX_TAYLOR_DEPTH = 10;
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

#define DFR_DUMP(x, ...) _dfr_dump (x, dfr_default_dump_file_name, __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)


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


#define dfr_sorry_message _dfr_sorry_message (__FILE__, __PRETTY_FUNCTION__, __LINE__)
//--------------------------------------------------


typedef struct Dfr_structure      Dfr;
typedef struct Dfr_info_structure Dfr_info;

struct         Dfr_info_structure  {

    const  char* name;
    Dfr*         adress;
    const  char* birth_file;
    const  char* birth_func;
    int          birth_line;
};


enum Buffer_operation_code {

    DBOC_OPEN_BRACKET,
    DBOC_CLOSING_BRACKET,
    DBOC_ADD,
    DBOC_SUB,
    DBOC_MULT,
    DBOC_DIV,
    DBOC_POW,
    DBOC_LN,
    DBOC_SIN,
    DBOC_COS,
    DBOC_TG,
    DBOC_CTG,
};


typedef struct Buffer_node {

    Atom_type atom_type;

    union {

        Buffer_operation_code val_buffer_operation_code;
        double val_double;
        char*  val_str;
    };

} Buffer_node; const size_t BUFFER_NODE_SIZE = sizeof (Buffer_node);

typedef struct Dfr_buffer {

    Buffer_node* buffer;
    size_t len;

} Dfr_buffer; const size_t DFR_BUFFER_SIZE = sizeof (Dfr_buffer);


struct Dfr_structure {

    Tree*  user_function_tree;
    Tree** derivative_trees_array;
    Tree** taylor;
    Tree*  tangent;

    Dfr_buffer* buffer;


    Dfr_info debug_info;

}; const size_t DFR_SIZE = sizeof (Dfr);


typedef struct Node_substitution {

    Node*  node;
    size_t substitution_number;

} Node_substitution; const size_t NODE_SUBSTITUTION_SIZE = sizeof (Node_substitution);


typedef struct Tree_substitution {

    Node_substitution* substitution_list;
    size_t num_substitutions;

} Tree_substitution; const size_t TREE_SUBSTITUTION_SIZE = sizeof (Tree_substitution);



//--------------------------------------------------
const size_t MAX_DERIVATIVE_NUM = fmax (MAX_TAYLOR_DEPTH, 1) + 1; //place for at least function and 1st derivative
const size_t MAX_PREAMBLE_LEN   = ceil (log (MAX_TAYLOR_DEPTH) / log (10)) + strlen ("'th derivative:        ");
const size_t MAX_PHRASE_LEN     = 100;
const size_t MAX_SUBSTITUTIONS  = 400;
const size_t MAX_TEX_LEN        = 80;
const size_t ALPHABET_LEN       = 24;
//--------------------------------------------------



Return_code _dfr_ctor                             (Dfr* dfr, const char* name, const char* file, const char* func, int line);
Return_code _dfr_ctor_fill_tree_array_with_poison (Tree** array, size_t len);


Return_code dfr_user_function_tree_ctor (Dfr* dfr);

Return_code dfr_dtor                  (Dfr* dfr);
Return_code dfr_derivative_trees_dtor (Dfr* dfr);
Return_code dfr_taylor_dtor           (Dfr* dfr);



void _dfr_dump       (Dfr* dfr, const char* file_name, const char* file, const char* function, int line,                        const char* additional_text = "");
void _fdfr_dump      (Dfr* dfr, const char* file_name, const char* file, const char* function, int line, const char* file_mode, const char* additional_text = "");
void _fdfr_graphdump (Dfr* dfr, const char* file_name, const char* file, const char* function, int line,                        const char* additional_text = "");

size_t get_operation_priority (Operation_code operation_code);
bool   are_associative        (Operation_code op_left, Operation_code op_right);


Return_code read_number   (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr);
Return_code read_variable (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr);
Return_code read_primary  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr);
Return_code read_unary    (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr);
Return_code read_power    (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr);
Return_code read_product  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr);
Return_code read_sum      (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr);
Return_code read_general  (Dfr_buffer* dfr_buffer, Tree* tree);

bool is_variable_start (char simbol);
bool is_variable_mid   (char simbol);
bool is_unary          (Node* node);
bool is_unary_minus    (Node* node);


Return_code dfr_read_user_function             (Dfr* dfr, const char* file_name = dfr_default_input_file_name);

Return_code dfr_write_user_function (Dfr*  dfr, int precision,                           const char* file_name = dfr_default_output_file_name);
Return_code dfr_write_derivative    (Dfr* dfr, int precision, size_t derivative_num = 1, const char* file_name = dfr_default_output_file_name);
const char* ordinal_ending          (size_t n);
Return_code dfr_write_taylor        (Dfr* dfr, const char* variable, size_t depth, int precision, const char* file_name = dfr_default_output_file_name);



Return_code write_text_function      (const char* file_name, const char* format, ...);
Return_code write_function           (Tree* tree, int precision, const char* file_name, const char* file_mode = "unknown");
Return_code write_function_dive_left (FILE* file, Tree* tree, Tree_iterator* tree_iterator);
Return_code write_function_inc       (FILE* file, Tree* tree, Tree_iterator* tree_iterator);

Return_code write_function_check_open_bracket    (FILE* file, Tree_iterator* tree_iterator, const char* next_node_str);
Return_code write_function_check_closing_bracket (FILE* file, Tree_iterator* tree_iterator);

Return_code tex_write_tree      (FILE* file, Tree* tree, int precitsion);
Return_code tex_write_node      (FILE* file, Node* node, Tree_substitution* substitutions, int precision);
Return_code tex_write_operation (FILE* file, Node* node, Tree_substitution* substitutions, int precision);
Return_code tex_write_const     (FILE* file, Node* node,                                   int precision);
Return_code tex_write_check_open_bracket    (FILE* file, Node* left, Node* right, Tree_substitution* substitutions);
Return_code tex_write_check_closing_bracket (FILE* file, Node* left, Node* right, Tree_substitution* substitutions);

Return_code tex_write_evaluation_introduction    (FILE* file, size_t derivative_num, double value, int precision);
Return_code tex_write_evaluation_ending          (FILE* file, size_t derivative_num, double value, Tree* answer, int precision);
Return_code tex_write_taylor_introduction        (FILE* file,           size_t depth, const char* variable, double point, int precision);
Return_code tex_write_taylor_ending              (FILE* file, Dfr* dfr, size_t depth, const char* variable, double point, int precision);
Return_code tex_write_derivative_introduction    (FILE* file, const char* variable, size_t derivative_num);
Return_code tex_write_derivative_ending          (FILE* file, Dfr* dfr, const char* variable, size_t derivative_num, int precision);
Return_code tex_write_user_function_introduction (FILE* file, const char* variable);
Return_code tex_write_user_function_ending       (FILE* file, Dfr* dfr, const char* variable, int precision);


const char* tex_get_phrase          (void);
Return_code tex_write_function_name (FILE* file, size_t derivative_num);

Return_code tex_generate_output     (Dfr* dfr, const char* variable, double taylor_point, size_t depth, double tangent_point, int precision, const char* file_name = dfr_default_tex_file_name);
Return_code tex_write_preamble      (FILE* file);
Return_code tex_write_end           (FILE* file);
Return_code tex_write_introduction  (FILE* file, Dfr* dfr, const char* variable, int precision);
Return_code tex_write_user_function (FILE* file, Dfr* dfr, const char* variable, int precision);
Return_code tex_write_derivative    (FILE* file, Dfr* dfr, const char* variable, int precision);
Return_code tex_write_taylor        (FILE* file, Dfr* dfr, const char* variable, double point, size_t depth, int precision);


Return_code node_get_tex_len      (Node* node, double* len_ptr, double len_coefficient, Tree_substitution* substitution_list, int precision);
Return_code double_get_tex_len    (double value, double* len_ptr, double len_coefficient, int precision);
size_t      size_t_get_tex_len    (size_t value);
Return_code variable_get_tex_len  (const char* variable, double* len_ptr, double len_coefficient);
Return_code operation_get_tex_len (Node* node, double* len_ptr, double len_coefficient, Tree_substitution* substitutions, int precision);
double      get_operation_tex_len (Operation_code operation_code, double len_coefficient);


Node* copy_node (Node* node);
Tree* copy_tree (Tree* tree);


Return_code dfr_calculate_derivative       (Dfr* dfr,   const char* variable, size_t derivative_num = 1, bool silent = true, FILE* file = nullptr, int precision = -1);
Node*       node_calculate_derivative      (Node* node, const char* variable);
Node*       operation_calculate_derivative (Node* node, const char* variable);
Node*       variable_calculate_derivative  (Node* node, const char* variable);

Return_code tree_evaluate            (Tree* tree, Tree* answer, const char* variable, double value, bool silent = true, FILE* file = nullptr, int precision = -1);
Return_code tree_substitute_variable (Tree* tree,               const char* variable, double value);
Return_code node_substitute_variable (Node* node,               const char* variable, double value);

Return_code  dfr_calculate_taylor  (Dfr* dfr, const char* variable, double point, size_t depth, bool silent = true, FILE* file = nullptr, int precision = -1);
Return_code  dfr_ensure_derivative (Dfr* dfr, const char* variable, size_t n, bool silent = true, FILE* file = nullptr);
double       factorial             (size_t n);


Return_code tree_fold (Tree* tree, bool silent = true, FILE* file = nullptr, int precision = -1);

Return_code tree_fold_constants      (Tree* tree, bool* folded_anything);
Return_code node_fold_constants      (Node* node, bool* folded_anything);
Return_code operation_fold_constants (Node* node, bool* folded_anything);

Return_code tree_fold_neutral      (Tree* tree, bool* folded_anything);
Return_code node_fold_neutral      (Node* node, bool* folded_anything);
Return_code operation_fold_neutral (Node* node, bool* folded_anything);

Return_code  dfr_buffer_ctor       (Dfr_buffer* dfr_buffer);
Return_code  dfr_buffer_dtor       (Dfr_buffer* dfr_buffer);
Return_code  dfr_buffer_dump       (Dfr_buffer* buffer);
Return_code _dfr_buffer_read       (Dfr_buffer* dfr_buffer, char* str);
Return_code  read_buffer_operation (char** str_ptr, Buffer_node* buffer_node);
Return_code  read_buffer_variable  (char** str_ptr, Buffer_node* buffer_node);
Return_code  read_buffer_number    (char** str_ptr, Buffer_node* buffer_node);
Return_code  skipspaces            (char** str_ptr);


Return_code        tree_substitution_ctor           (Tree_substitution* tree_substitution);
Return_code        tree_substitution_dtor           (Tree_substitution* tree_substitution);
Return_code        tree_substitute                  (Tree* tree, Tree_substitution* tree_substitution,                             int precision);
Return_code        node_substitute                  (Node* node, Tree_substitution* tree_substitution, double cur_len_coefficient, int precision);
Return_code        node_substitute_left             (Node* node, Tree_substitution* tree_substitution, double cur_len_coefficient, int precision);
Return_code        node_substitute_right            (Node* node, Tree_substitution* tree_substitution, double cur_len_coefficient, int precision);
Return_code        tree_add_substitution            (Tree_substitution* tree_substitution, Node* node);
double             get_tex_substitution_len         (Tree_substitution* substitution_list, Node* node);
double             get_substitution_letter_len      (size_t substitution_number);
Node_substitution* get_tex_substitution             (Tree_substitution* substitutions, Node* node);
Return_code        tex_write_substitution           (FILE* file, Node_substitution* substitution);
Return_code        tex_write_substitution_letter    (FILE* file, Node_substitution* substitution);
Return_code        tex_write_substitution_index     (FILE* file, Node_substitution* substitution);
Return_code        tex_write_substitutions_decoding (FILE* file, Tree_substitution* substitutions, int precision);
Return_code        tex_write_substitution_decoding  (FILE* file, Tree_substitution* substitutions, size_t index, int precision);


bool trees_the_same            (Tree* tree1, Tree* tree2);
bool nodes_the_same            (Node* node1, Node* node2);
bool operation_nodes_the_same  (Node* node1, Node* node2);
bool sons_the_same             (Node* node1, Node* node2, bool is_symmetric_operation);


Return_code dfr_generate_gnu_plot_describtion (Dfr* dfr, const char* variable, int precitsion, const char* file_name = dfr_gnu_plot_describtion_file_name);
Return_code dfr_calculate_tangent             (Dfr* dfr, const char* variable, double point);
Return_code dfr_generate_graph                (            Dfr* dfr, const char* variable, double point, int precision);
Return_code tex_write_graph                   (FILE* file, Dfr* dfr, const char* variable, double point, int precision);

Return_code tex_generate_pdf (const char* file_name = dfr_default_tex_file_name);

Return_code _dfr_sorry_message (const char* file, const char* func, int line);

/*

Return_code  dfr_save (Tree* tree, const char* file_name = tree_default_save_file_name);
Return_code  dfr_read (Tree* tree, const char* file_name = tree_default_save_file_name);
Return_code _dfr_read (char* buffer, size_t* current, Tree* tree, Node* node);

*/


//--------------------------------------------------
#endif