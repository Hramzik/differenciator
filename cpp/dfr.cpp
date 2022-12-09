

#include "../lib/tree_dfr.hpp"

#include "../lib/double_compare.hpp"

#include "../headers/dfr.hpp"


//--------------------------------------------------


Return_code  _dfr_ctor  (Dfr* dfr, const char* name, const char* file, const char* func, int line) {

    assert ( (file) && (name) && (func) && (line > 0) );
    if (dfr == nullptr) { LOG_ERROR (BAD_ARGS); DFR_ERROR_DUMP (dfr); return BAD_ARGS; }


    dfr->buffer = (Dfr_buffer*) calloc (DFR_BUFFER_SIZE, 1);
    dfr_buffer_ctor (dfr->buffer);

    dfr->user_function_tree = nullptr;
    dfr->tangent            = nullptr;


    dfr->derivative_trees_array = (Tree**) calloc (sizeof (Tree*) * MAX_DERIVATIVE_NUM, 1);
    _dfr_ctor_fill_tree_array_with_poison (dfr->derivative_trees_array, MAX_DERIVATIVE_NUM);
    dfr->taylor                 = (Tree**) calloc (sizeof (Tree*) * (MAX_TAYLOR_DEPTH + 1), 1);
    _dfr_ctor_fill_tree_array_with_poison (dfr->taylor, MAX_TAYLOR_DEPTH + 1);


    dfr->debug_info.name       = name;
    dfr->debug_info.birth_file = file;
    dfr->debug_info.birth_func = func;
    dfr->debug_info.birth_line = line;
    dfr->debug_info.adress     = dfr;


    DFR_AFTER_OPERATION_DUMPING (dfr);


    return SUCCESS;
}


Return_code  dfr_user_function_tree_ctor  (Dfr* dfr) {

    if (!dfr || dfr->user_function_tree || !dfr->derivative_trees_array || dfr->derivative_trees_array [0]) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    dfr->user_function_tree = (Tree*) calloc (TREE_SIZE, 1);
    TREE_CTOR (dfr->user_function_tree);

    dfr->derivative_trees_array [0] = (Tree*) calloc (TREE_SIZE, 1);
    TREE_CTOR (dfr->derivative_trees_array [0]);


    return SUCCESS;
}


Return_code  _dfr_ctor_fill_tree_array_with_poison  (Tree** array, size_t len) {

    if (!array) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < len; i++) {
    
        array [i] = nullptr;
    }


    return SUCCESS;
}


Return_code  dfr_dtor  (Dfr* dfr) {

    if (!dfr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if (dfr->user_function_tree) { try (tree_dtor (dfr->user_function_tree)); free (dfr->user_function_tree); dfr->user_function_tree = nullptr; }
    if (dfr->tangent)            { try (tree_dtor (dfr->tangent));             free (dfr->tangent);            dfr->tangent            = nullptr; }


    try (dfr_derivative_trees_dtor (dfr));


    try (dfr_taylor_dtor (dfr));


    try (dfr_buffer_dtor (dfr->buffer));
    dfr->buffer = nullptr;


    return SUCCESS;
}


Return_code  dfr_derivative_trees_dtor  (Dfr* dfr) {

    if (!dfr || !dfr->derivative_trees_array) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < MAX_DERIVATIVE_NUM; i++) {
    
        if (dfr->derivative_trees_array [i]) {

            tree_dtor (dfr->derivative_trees_array [i]);
        }
    }


    return SUCCESS;
}


Return_code  dfr_taylor_dtor  (Dfr* dfr) {

    if (!dfr || !dfr->taylor) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < MAX_TAYLOR_DEPTH; i++) {
    
        if (dfr->taylor [i]) {

            tree_dtor (dfr->taylor [i]);
        }
    }


    return SUCCESS;
}


void  _dfr_dump  (Dfr* dfr, const char* file_name, const char* file, const char* function, int line, const char* additional_text) {

    static bool first_time_dumping = true;


    const char* file_mode = nullptr;

    if (first_time_dumping) { file_mode = "w"; }
    else                    { file_mode = "a"; }


    _fdfr_dump (dfr, file_name, file, function, line, file_mode, additional_text);


    first_time_dumping = false;


    return;
}


void  _fdfr_dump  (Dfr* dfr, const char* file_name, const char* file, const char* function, int line, const char* file_mode, const char* additional_text) {


    FILE* dump_file = fopen (file_name, file_mode);
    if (dump_file == nullptr) { LOG_ERROR (FILE_ERR); return; }


    setvbuf (dump_file, NULL, _IONBF, 0);


    fprintf (dump_file, "<pre><h1>");
    fprintf (dump_file,"%s", additional_text);
    fprintf (dump_file, "</h1>");
    fprintf (dump_file, "<h1>Dumping differenciator at %s in function %s (line %d)...</h1>\n\n", file, function, line);


    if (!dfr) { fprintf (dump_file, "Differenciator pointer is nullptr!\n\n"); return; }


    fprintf (dump_file, "this differenciator has name ");
    if (dfr->debug_info.name != nullptr) { fprintf (dump_file, "%s ", dfr->debug_info.name); }
    else                                 { fprintf (dump_file, "UNKNOWN NAME "); }
    fprintf (dump_file, "[%p]\n", dfr->debug_info.adress);

    fprintf (dump_file, "it was created in file ");
    if (dfr->debug_info.birth_file != nullptr) { fprintf (dump_file, "%s\n", dfr->debug_info.birth_file); }
    else                                       { fprintf (dump_file, "UNKNOWN NAME\n"); }

    fprintf (dump_file, "in function ");
    if (dfr->debug_info.birth_func != nullptr) { fprintf (dump_file, "%s ", dfr->debug_info.birth_func); }
    else                                       { fprintf (dump_file, "UNKNOWN NAME "); }

    fprintf (dump_file, "(line %d)\n\n", dfr->debug_info.birth_line);


    fclose (dump_file);


    return;
}


size_t  get_operation_priority  (Operation_code operation_code) {

    switch (operation_code) {

        case DOC_UNKNOWN: return 0;
        case DOC_ADD:  return 3;
        case DOC_SUB:  return 3;
        case DOC_DIV:  return 4;
        case DOC_MULT: return 4;
        case DOC_POW:  return 5;
        case DOC_LN:   return 6;
        case DOC_SIN:  return 6;
        case DOC_COS:  return 6;
        case DOC_TG:   return 6;
        case DOC_CTG:  return 6;

        default: LOG_ERROR (BAD_ARGS); return 0;
    }
}


Return_code  read_number  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || ((*buffer_node_ptr)->atom_type != DAT_CONST)
                         || (*buffer_node_ptr >= max_buffer_node) || !node_ptr || !isfinite ((*buffer_node_ptr)->val_double) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    (*node_ptr) = create_node (DAT_CONST, (*buffer_node_ptr)->val_double);


    *buffer_node_ptr += 1;


    return SUCCESS;
}


bool  is_variable_start  (char simbol) {

    if ((simbol >= 'a' && simbol <= 'z') ||
        (simbol >= 'A' && simbol <= 'Z') ||
        (simbol == '_')) {

        return true;
    }


    return false;
}


bool  is_variable_mid  (char simbol) {

    if ((simbol >= '0' && simbol <= '9') ||
        (simbol >= 'a' && simbol <= 'z') ||
        (simbol >= 'A' && simbol <= 'Z') ||
        (simbol == '_')) {

        return true;
    }


    return false;
}


Return_code  read_variable  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || ((*buffer_node_ptr)->atom_type != DAT_VARIABLE)
                         || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    (*node_ptr) = create_node (DAT_VARIABLE, (*buffer_node_ptr)->val_str);


    *buffer_node_ptr += 1;


    return SUCCESS;
}


//--------------------------------------------------
#define CHECK_CLOSING_BRACKET\
    if ( (*buffer_node_ptr)->atom_type != DAT_OPERATION || (*buffer_node_ptr)->val_buffer_operation_code != DBOC_CLOSING_BRACKET) {\
\
        LOG_ERROR (BAD_ARGS);\
        return BAD_ARGS;\
    }
//--------------------------------------------------


Return_code  read_primary  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if ( (*buffer_node_ptr)->atom_type == DAT_OPERATION &&  (*buffer_node_ptr)->val_buffer_operation_code == DBOC_OPEN_BRACKET) {

        *buffer_node_ptr += 1;

        try (read_sum (buffer_node_ptr, max_buffer_node, node_ptr));

        CHECK_CLOSING_BRACKET

        *buffer_node_ptr += 1;
    }

    else if ( (*buffer_node_ptr)->atom_type == DAT_VARIABLE ) { try (read_variable (buffer_node_ptr, max_buffer_node, node_ptr)); }
    else                                                      { try (read_number   (buffer_node_ptr, max_buffer_node, node_ptr)); }


    return SUCCESS;
}


//--------------------------------------------------
#undef CHECK_CLOSING_BRACKET
//--------------------------------------------------


//--------------------------------------------------
#define CURRENT (*buffer_node_ptr)
#define CUR_OP CURRENT->val_buffer_operation_code
//--------------------------------------------------


Return_code  read_unary  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Node* new_node = nullptr;

    if ( CURRENT->atom_type == DAT_OPERATION && (CUR_OP == DBOC_SIN ||
                                                 CUR_OP == DBOC_COS ||
                                                 CUR_OP == DBOC_TG  ||
                                                 CUR_OP == DBOC_CTG ||
                                                 CUR_OP == DBOC_SUB ||
                                                 CUR_OP == DBOC_LN)) {

        switch (CUR_OP) {

            case DBOC_SIN: new_node = create_node (DAT_OPERATION, DOC_SIN); break;
            case DBOC_COS: new_node = create_node (DAT_OPERATION, DOC_COS); break;
            case DBOC_TG:  new_node = create_node (DAT_OPERATION, DOC_TG);  break;
            case DBOC_CTG: new_node = create_node (DAT_OPERATION, DOC_CTG); break;
            case DBOC_SUB: new_node = create_node (DAT_OPERATION, DOC_SUB); break;
            case DBOC_LN:  new_node = create_node (DAT_OPERATION, DOC_LN);  break;

            case DBOC_OPEN_BRACKET:    LOG_ERROR (BAD_ARGS); return BAD_ARGS;
            case DBOC_CLOSING_BRACKET: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
            case DBOC_ADD:             LOG_ERROR (BAD_ARGS); return BAD_ARGS;
            case DBOC_MULT:            LOG_ERROR (BAD_ARGS); return BAD_ARGS;
            case DBOC_DIV:             LOG_ERROR (BAD_ARGS); return BAD_ARGS;
            case DBOC_POW:             LOG_ERROR (BAD_ARGS); return BAD_ARGS;
            default:                   LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        }


        new_node->left_son = create_node (DAT_CONST, (double) 0);


        *node_ptr = new_node;


        CURRENT += 1;


        try (read_primary (buffer_node_ptr, max_buffer_node, &(*node_ptr)->right_son ));


        return SUCCESS;
    }


    try (read_primary (buffer_node_ptr, max_buffer_node, node_ptr));


    return SUCCESS;
}


//--------------------------------------------------
#undef CURRENT
#undef CUR_OP
//--------------------------------------------------


Return_code  read_power  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (read_unary (buffer_node_ptr, max_buffer_node, node_ptr));


    Node* new_node = nullptr;

    while ( (*buffer_node_ptr)->atom_type == DAT_OPERATION && ( (*buffer_node_ptr)->val_buffer_operation_code == DBOC_POW) ) {

        new_node = create_node (DAT_OPERATION, DOC_POW);
        new_node->left_son = *node_ptr;


        *buffer_node_ptr += 1;

        *node_ptr = new_node;

        try (read_unary (buffer_node_ptr, max_buffer_node, &(*node_ptr)->right_son));
    }


    return SUCCESS;
}


Return_code  read_product  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (read_power (buffer_node_ptr, max_buffer_node, node_ptr));


    Node* new_node = nullptr;

    while ( (*buffer_node_ptr)->atom_type == DAT_OPERATION && ( (*buffer_node_ptr)->val_buffer_operation_code == DBOC_MULT ||
                                                                (*buffer_node_ptr)->val_buffer_operation_code == DBOC_DIV)) {


        if ((*buffer_node_ptr)->val_buffer_operation_code == DBOC_MULT) {

            new_node = create_node (DAT_OPERATION, DOC_MULT);
            new_node->left_son = *node_ptr;
        }

        else {

            new_node = create_node (DAT_OPERATION, DOC_DIV);
            new_node->left_son = *node_ptr;
        }

        *buffer_node_ptr += 1;

        *node_ptr = new_node;

        try (read_power (buffer_node_ptr, max_buffer_node, &(*node_ptr)->right_son));
    }


    return SUCCESS;
}


Return_code  read_sum  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (read_product (buffer_node_ptr, max_buffer_node, node_ptr));


    Node* new_node = nullptr;

    while ( (*buffer_node_ptr)->atom_type == DAT_OPERATION && ( (*buffer_node_ptr)->val_buffer_operation_code == DBOC_ADD ||
                                                                (*buffer_node_ptr)->val_buffer_operation_code == DBOC_SUB)) {


        if ((*buffer_node_ptr)->val_buffer_operation_code == DBOC_ADD) {

            new_node = create_node (DAT_OPERATION, DOC_ADD);
            new_node->left_son = *node_ptr;
        }

        else {

            new_node = create_node (DAT_OPERATION, DOC_SUB);
            new_node->left_son = *node_ptr;
        }

        *buffer_node_ptr += 1;

        *node_ptr = new_node;

        try (read_product (buffer_node_ptr, max_buffer_node, &(*node_ptr)->right_son));
    }


    return SUCCESS;
}


Return_code  read_general  (Dfr_buffer* dfr_buffer, Tree* tree) {

    if (!dfr_buffer || !tree) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Buffer_node* current_buffer_node = dfr_buffer->buffer;


    try (read_sum (&current_buffer_node, dfr_buffer->buffer + dfr_buffer->len, &tree->root));


    if (current_buffer_node != (dfr_buffer->buffer + dfr_buffer->len) ) return BAD_ARGS;


    return SUCCESS;
}


Return_code  dfr_read_user_function  (Dfr* dfr, const char* file_name) {

    if (!dfr || !file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    FILE* file = fopen (file_name, "r");
    if (!file) { LOG_ERROR (FILE_ERR); return FILE_ERR; }


    size_t file_len = get_file_len (file);
    char buffer [file_len + 1] = "";
    fread (buffer, 1, file_len, file);
    fclose (file);


    _dfr_buffer_read (dfr->buffer, buffer);


    //dfr_buffer_dump (dfr->buffer);


    dfr_user_function_tree_ctor (dfr);


    try (read_general (dfr->buffer, dfr->user_function_tree));


    return SUCCESS;
}


/*
void  _fdfr_graphdump  (Dfr* dfr, const char* file_name, const char* file, const char* function, int line, const char* additional_text) {

    assert ( (file_name) && (file) && (function) && (line > 0) && (additional_text));


    char file_path [MAX_COMMAND_LEN] = "";
    strcat (file_path, file_name);
    strcat (file_path, ".html");


    const char* file_mode = nullptr;
    if ( !strcmp (GLOBAL_graph_dump_num, "1")) { file_mode = "w"; }
    else                                       { file_mode = "a"; }


    printf ("(dfr):  generating %s graph dump...\n", GLOBAL_graph_dump_num);


    _fdfr_dump (dfr, file_path, file, function, line, file_mode, additional_text);


    itoa ( (atoi (GLOBAL_graph_dump_num) + 1), GLOBAL_graph_dump_num, DEFAULT_COUNTING_SYSTEM); //incrementation of graph_dump_num


    FTREE_GRAPHDUMP (dfr->user_function_tree, "<hr>User function tree:\n");
    FTREE_GRAPHDUMP (dfr->derivative_tree,       "<hr>Derivative tree:\n");


    return;
}
*/

Return_code  dfr_write_user_function  (Dfr* dfr, int precision, const char* file_name) {

    if (!dfr || !file_name || !dfr->user_function_tree || !dfr->derivative_trees_array || !dfr->derivative_trees_array [0]) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (write_text_function (file_name, "%s", "your function:          "));
    try (write_function (dfr->user_function_tree, precision, file_name));
    try (write_text_function (file_name, "%s", "\nyour function (folded): "));
    try (write_function (dfr->derivative_trees_array [0], precision, file_name));


    return SUCCESS;
}


Return_code  dfr_write_derivative  (Dfr* dfr, int precision, size_t derivative_num, const char* file_name) {

    if (!dfr || !file_name || !dfr->derivative_trees_array || !dfr->derivative_trees_array [derivative_num]) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    if (derivative_num > MAX_TAYLOR_DEPTH) {

        LOG_MESSAGE ("derivative num must be less than MAX_TAYLOR_LEN");
        LOG_ERROR (BAD_ARGS);
        return BAD_ARGS;
    }


    char preamble [MAX_PREAMBLE_LEN + 1] = "";
    sprintf (preamble, "\n%zd%s derivative:        ", derivative_num, ordinal_ending (derivative_num));


    try (write_text_function (file_name, preamble))
    try (write_function (dfr->derivative_trees_array [derivative_num], precision, file_name));


    return SUCCESS;
}


Return_code  dfr_write_taylor  (Dfr* dfr, const char* variable, size_t depth, int precision, const char* file_name) {

    if (!dfr || !file_name || !dfr->taylor) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (write_text_function (file_name, "\ntaylor:                 "))
    try (write_function (dfr->taylor [0], precision, file_name));


    for (size_t i = 1; i <= depth; i++) {

        try (write_text_function (file_name, " + ("));
        try (write_function (dfr->taylor [i], precision, file_name));
        try (write_text_function (file_name, ") * %s^%zd", variable, i));

    }


    try (write_text_function (file_name, " + o(%s^%zd)", variable, depth));


    return SUCCESS;
}


const char*  ordinal_ending (size_t n) {

    switch (n) {

        case 1: return "'st";
        case 2: return "'nd";
        case 3: return "'d";

        default: return "'th";
    }
}


Return_code  write_text_function (const char* file_name, const char* format, ...) {

    if (!file_name || !format) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    FILE* file = fopen (file_name, "a");
    if (file == nullptr) { LOG_ERROR (FILE_ERR); return FILE_ERR; }


    va_list args;
    va_start (args, format);


    vfprintf (file, format, args);


    va_end (args);


    fclose (file);


    return SUCCESS;
}


Return_code  write_function  (Tree* tree, int precision, const char* file_name, const char* file_mode) {

    if (!tree || !file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    static bool first_time_writing = true;


    if (strcmp (file_mode, "w") && strcmp (file_mode, "a")) {

        if (first_time_writing) { file_mode = "w"; }
        else                    { file_mode = "a"; }
    }


    FILE* file = fopen (file_name, file_mode);
    if (file == nullptr) { LOG_ERROR (FILE_ERR); return FILE_ERR; }


    Tree_iterator tree_iterator = {};
    tree_iterator_ctor (&tree_iterator, tree, "my_mode");


    write_function_dive_left (file, tree, &tree_iterator);


    do {

        switch (tree_iterator.current->atom_type) {

            case DAT_OPERATION: fprintf (file, " %s ", _operation_code_to_str (tree_iterator.current->element.value.val_operation_code)); break;
            case DAT_CONST:     fprintf (file, "%.*lf", precision,             tree_iterator.current->element.value.val_double);          break;
            case DAT_VARIABLE:  fprintf (file, "%s",                           tree_iterator.current->element.value.var_str);             break;

            default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        }

    } while (!write_function_inc (file, tree, &tree_iterator));


    first_time_writing = false;


    fclose (file);


    return SUCCESS;
}


Return_code  write_function_dive_left  (FILE* file, Tree* tree, Tree_iterator* tree_iterator) {

    if (!tree || !tree_iterator || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    stack_push (tree_iterator->node_stack, tree_iterator->current);
    tree_iterator->depth = 0;


    while (tree_iterator->current->left_son && !is_unary (tree_iterator->current)) {

        write_function_check_open_bracket (file, tree_iterator, "L");


        tree_iterator->current = tree_iterator->current->left_son;
        stack_push (tree_iterator->node_stack, tree_iterator->current);
        tree_iterator->depth += 1;
    }


    return SUCCESS;
}


Return_code  write_function_inc  (FILE* file, Tree* tree, Tree_iterator* tree_iterator) {

    if (!tree || !tree_iterator || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (tree_iterator->current->right_son) {

        write_function_check_open_bracket (file, tree_iterator, "R");


        tree_iterator->current = tree_iterator->current->right_son;
        stack_push (tree_iterator->node_stack, tree_iterator->current);
        tree_iterator->depth += 1;

        while (tree_iterator->current->left_son && !is_unary (tree_iterator->current) ) {

            write_function_check_open_bracket (file, tree_iterator, "L");


            tree_iterator->current = tree_iterator->current->left_son;
            stack_push (tree_iterator->node_stack, tree_iterator->current);
            tree_iterator->depth += 1;
        }

        tree_iterator->index += 1;
        return SUCCESS;
    }


    if (tree_iterator->depth == 0) { return BAD_ARGS; } //дошли до корня, справа нет узлов


    write_function_check_closing_bracket (file, tree_iterator);


    Node* old = tree_iterator->current;
    stack_pop (tree_iterator->node_stack);
    tree_iterator->current = (Node*) stack_pop (tree_iterator->node_stack).value;
    tree_iterator->depth -= 1;
    stack_push (tree_iterator->node_stack, tree_iterator->current);



    while (tree_iterator->current->right_son == old) {//пришел справа

        if (tree_iterator->depth == 0) { return BAD_ARGS; } // already was at last element


        write_function_check_closing_bracket (file, tree_iterator);


        old = tree_iterator->current;
        stack_pop (tree_iterator->node_stack);

        tree_iterator->current = (Node*) stack_pop (tree_iterator->node_stack).value;
        stack_push (tree_iterator->node_stack, tree_iterator->current);
        tree_iterator->depth -= 1;
    }

//пришел слева
    tree_iterator->index += 1;
    return SUCCESS;
}


//pattern: c1 op_left (c2 op_right c3), can we leave the parenthesis?
bool  are_associative  (Operation_code op_left, Operation_code op_right) {

    if (get_operation_priority (op_left) < get_operation_priority (op_right)) return true;
    if (get_operation_priority (op_left) > get_operation_priority (op_right)) return false;


    //same priority case:
    if (op_left == DOC_ADD) {

        if (op_right == DOC_ADD || op_right == DOC_SUB)  return true;
    }

    if (op_left == DOC_MULT) {

        if (op_right == DOC_MULT || op_right == DOC_DIV) return true;
    }


    return false;
}


Return_code  write_function_check_open_bracket  (FILE* file, Tree_iterator* tree_iterator, const char* next_node_str) {

    if (!tree_iterator || !file || !next_node_str || (stricmp (next_node_str, "L") && stricmp (next_node_str, "R")) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if ( is_unary (tree_iterator->current)) { fprintf (file, "("); return SUCCESS; }



    Node* son = nullptr;

    if (!stricmp (next_node_str, "L")) { son = tree_iterator->current->left_son;  }
    else                               { son = tree_iterator->current->right_son; }


    //if ( is_unary_minus (son)) { fprintf (file, "("); return SUCCESS; }


    if (tree_iterator->current->atom_type == DAT_OPERATION && son->atom_type == DAT_OPERATION &&
        !are_associative (tree_iterator->current->element.value.val_operation_code, son->element.value.val_operation_code)) {

        fprintf (file, "(");
    }


    return SUCCESS;
}


Return_code  write_function_check_closing_bracket  (FILE* file, Tree_iterator* tree_iterator) {

    if (!tree_iterator || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Node* son    = (Node*) stack_pop  (tree_iterator->node_stack).value;
    Node* parent = (Node*) stack_pop  (tree_iterator->node_stack).value;
                           stack_push (tree_iterator->node_stack, parent);
                           stack_push (tree_iterator->node_stack, son);


    if ( is_unary       (parent)) { fprintf (file, ")"); return SUCCESS; }
    //if ( is_unary_minus (son))    { fprintf (file, ")"); return SUCCESS; }


    if (son->atom_type == DAT_OPERATION && parent->atom_type == DAT_OPERATION &&
        !are_associative (parent->element.value.val_operation_code, son->element.value.val_operation_code)) {

        fprintf (file, ")");
    }


    return SUCCESS;
}


Return_code  dfr_calculate_derivative  (Dfr* dfr, const char* variable, size_t derivative_num, bool silent, FILE* file, int precision) {

    if (!dfr || !variable) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    if (derivative_num == 0 || derivative_num > MAX_TAYLOR_DEPTH) {

        LOG_MESSAGE ("derivative number must be between 1 and MAX_TAYLOR_DEPTH!\n");
        LOG_ERROR (BAD_ARGS);
        return BAD_ARGS;
    }

    if (!dfr->derivative_trees_array [derivative_num - 1]) {

        LOG_MESSAGE ("to calculate %zd derivative, you must calculate %zd - 1 derivative first!\n");
        LOG_ERROR (BAD_ARGS);
        return BAD_ARGS;
    }

    if (!silent && !file) { LOG_MESSAGE ("can't write into nullptr file! (specify write file if you didn't)"); silent = true; }



    if (!silent) tex_write_derivative_introduction (file, variable, derivative_num);



    Tree* derivative_tree = (Tree*) calloc (TREE_SIZE, 1);
    TREE_CTOR      (derivative_tree);
    tree_kill_root (derivative_tree);


    derivative_tree->root = node_calculate_derivative (dfr->derivative_trees_array [derivative_num - 1]->root, variable);
    tree_fold (derivative_tree, silent, file, precision);




    dfr->derivative_trees_array [derivative_num] = derivative_tree;


    if (!silent) tex_write_derivative_ending (file, dfr, variable, derivative_num, precision);


    return SUCCESS;
}


Node*  node_calculate_derivative  (Node* node, const char* variable) {

    switch (node->atom_type) {

        case DAT_OPERATION: return operation_calculate_derivative (node, variable);
        case DAT_CONST:     return create_node (DAT_CONST, (double) 0);
        case DAT_VARIABLE:  return variable_calculate_derivative  (node, variable);

        default: LOG_ERROR (BAD_ARGS); return nullptr;
    }
}


//--------------------------------------------------
#define ROOT root_node

#define LEFT  root_node-> left_son
#define RIGHT root_node->right_son

#define LEFTLEFT   root_node-> left_son-> left_son
#define LEFTRIGHT  root_node-> left_son->right_son
#define RIGHTLEFT  root_node->right_son-> left_son
#define RIGHTRIGHT root_node->right_son->right_son
//--------------------------------------------------


Node*  operation_calculate_derivative  (Node* node, const char* variable) {

    if (!node || !variable || (node->atom_type != DAT_OPERATION)) { LOG_ERROR (BAD_ARGS); return nullptr; }


    Node* root_node = nullptr;

    switch (node->element.value.val_operation_code) {

        case DOC_UNKNOWN:

            LOG_ERROR (BAD_ARGS); root_node = create_node (DAT_OPERATION, DOC_UNKNOWN); break;

        case DOC_ADD:

            ROOT = create_node (DAT_OPERATION, DOC_ADD);

            LEFT  = node_calculate_derivative (node->left_son,  variable);
            RIGHT = node_calculate_derivative (node->right_son, variable);

            break;

        case DOC_SUB:

            ROOT = create_node (DAT_OPERATION, DOC_SUB);

            LEFT  = node_calculate_derivative (node->left_son,  variable);
            RIGHT = node_calculate_derivative (node->right_son, variable);

            break;

        case DOC_MULT:

            ROOT = create_node (DAT_OPERATION, DOC_ADD);

            LEFT  = create_node (DAT_OPERATION, DOC_MULT);
            RIGHT = create_node (DAT_OPERATION, DOC_MULT);

            LEFTLEFT   = node_calculate_derivative (node->left_son,  variable);
            LEFTRIGHT  = copy_node (node->right_son);
            RIGHTLEFT  = copy_node (node-> left_son);
            RIGHTRIGHT = node_calculate_derivative (node->right_son, variable);

            break;

        case DOC_DIV:

            ROOT = create_node (DAT_OPERATION, DOC_DIV);

            LEFT  = create_node (DAT_OPERATION, DOC_SUB);
            RIGHT = create_node (DAT_OPERATION, DOC_MULT);

            LEFTLEFT   = create_node (DAT_OPERATION, DOC_MULT); //numerator
            LEFTLEFT->left_son  = node_calculate_derivative (node->left_son, variable);
            LEFTLEFT->right_son = copy_node (node->right_son);
            LEFTRIGHT = create_node (DAT_OPERATION, DOC_MULT);
            LEFTRIGHT->left_son  = copy_node (node->left_son);
            LEFTRIGHT->right_son = node_calculate_derivative (node->right_son, variable);

            RIGHTLEFT  = copy_node (node->right_son);
            RIGHTRIGHT = copy_node (node->right_son);

            break;

        case DOC_POW:

            if (node->left_son->atom_type == DAT_CONST && node->right_son->atom_type == DAT_CONST) return create_node (DAT_CONST, (double) 0);

            else if (node->right_son->atom_type == DAT_CONST) {

                ROOT = create_node (DAT_OPERATION, DOC_MULT);

                LEFT = create_node (DAT_OPERATION, DOC_MULT);
                LEFTLEFT  = node_calculate_derivative (node->left_son, variable);
                LEFTRIGHT = copy_node (node->right_son);

                RIGHT = create_node (DAT_OPERATION, DOC_POW);
                RIGHTLEFT             = copy_node (node->left_son);
                RIGHTRIGHT            = create_node (DAT_OPERATION, DOC_SUB);
                RIGHTRIGHT-> left_son = copy_node (node->right_son);
                RIGHTRIGHT->right_son = create_node (DAT_CONST, (double) 1);
            }

            else if (node->left_son->atom_type == DAT_CONST) {

                ROOT = create_node (DAT_OPERATION, DOC_MULT);

                LEFT = create_node (DAT_OPERATION, DOC_MULT); //ln(a) * x'
                LEFTLEFT  = node_calculate_derivative (node->right_son, variable);
                LEFTRIGHT = create_node (DAT_OPERATION, DOC_LN);
                LEFTRIGHT-> left_son = create_node (DAT_CONST, (double) 0);
                LEFTRIGHT->right_son = copy_node (node->left_son);

                RIGHT = copy_node (node); //a**x
            }

            else {

                Node* temp = create_node (DAT_OPERATION, DOC_MULT); //g * ln(f)
                temp->left_son =  copy_node (node->right_son);
                temp->right_son = create_node (DAT_OPERATION, DOC_LN);
                temp->right_son->left_son  = create_node (DAT_CONST, (double) 0);
                temp->right_son->right_son = copy_node (node->left_son);

                ROOT  = create_node (DAT_OPERATION, DOC_MULT);
                LEFT  = copy_node (node);
                RIGHT = node_calculate_derivative (temp, variable);

                _node_dtor (temp);
            }

            break;

        case DOC_LN:

            ROOT = create_node (DAT_OPERATION, DOC_DIV);

            LEFT  = node_calculate_derivative (node->right_son, variable);
            RIGHT = copy_node (node->right_son);

            break;

        case DOC_SIN:

            ROOT = create_node (DAT_OPERATION, DOC_MULT);

            LEFT       = node_calculate_derivative (node->right_son, variable);
            RIGHT      = create_node (DAT_OPERATION, DOC_COS);
            RIGHTLEFT  = create_node (DAT_CONST, (double) 0);
            RIGHTRIGHT = copy_node (node->right_son);

            break;

        case DOC_COS:

            ROOT = create_node (DAT_OPERATION, DOC_MULT);

            LEFT       = create_node (DAT_OPERATION, DOC_MULT);
            LEFTLEFT   = create_node (DAT_CONST, (double) -1);
            LEFTRIGHT  = node_calculate_derivative (node->right_son, variable);
            RIGHT      = create_node (DAT_OPERATION, DOC_SIN);
            RIGHTLEFT  = create_node (DAT_CONST, (double) 0);
            RIGHTRIGHT = copy_node (node->right_son);

            break;

        case DOC_TG:

            ROOT = create_node (DAT_OPERATION, DOC_DIV);

            LEFT       = node_calculate_derivative (node->right_son, variable);

            RIGHT      = create_node (DAT_OPERATION, DOC_MULT);
            RIGHTLEFT  = create_node (DAT_OPERATION, DOC_COS);
            RIGHTRIGHT = create_node (DAT_OPERATION, DOC_COS);
            RIGHTLEFT-> left_son  = create_node (DAT_CONST, (double) 0);
            RIGHTRIGHT->left_son  = create_node (DAT_CONST, (double) 0);
            RIGHTLEFT-> right_son = copy_node (node->right_son);
            RIGHTRIGHT->right_son = copy_node (node->right_son);

            break;

        case DOC_CTG:

            ROOT = create_node (DAT_OPERATION, DOC_DIV);

            LEFT       = create_node (DAT_OPERATION, DOC_MULT);
            LEFTLEFT   = create_node (DAT_CONST, (double) -1);
            LEFTRIGHT  = node_calculate_derivative (node->right_son, variable);

            RIGHT      = create_node (DAT_OPERATION, DOC_MULT);
            RIGHTLEFT  = create_node (DAT_OPERATION, DOC_SIN);
            RIGHTRIGHT = create_node (DAT_OPERATION, DOC_SIN);
            RIGHTLEFT-> left_son  = create_node (DAT_CONST, (double) 0);
            RIGHTRIGHT->left_son  = create_node (DAT_CONST, (double) 0);
            RIGHTLEFT-> right_son = copy_node (node->right_son);
            RIGHTRIGHT->right_son = copy_node (node->right_son);

            break;

        default:

            LOG_ERROR (BAD_ARGS); return nullptr;
    }

    return root_node;
}


//--------------------------------------------------
#undef ROOT
#undef LEFT
#undef RIGHT
#undef LEFTLEFT
#undef LEFTRIGHT
#undef RIGHTLEFT
#undef RIGHTRIGHT
//--------------------------------------------------


Node*  variable_calculate_derivative  (Node* node, const char* variable) {

    if (!node || !variable || (node->atom_type != DAT_VARIABLE)) { LOG_ERROR (BAD_ARGS); return nullptr; }


    if ( !strcmp (node->element.value.var_str, variable) ) { return create_node (DAT_CONST, (double) 1); }


    return create_node (DAT_CONST, (double) 0);
}


Return_code  tree_fold_constants  (Tree* tree, bool* folded_anything) {

    if (!tree || !folded_anything) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (node_fold_constants (tree->root, folded_anything));


    return SUCCESS;
}


Return_code  node_fold_constants  (Node* node, bool* folded_anything) {

    if (!node || !folded_anything) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if (node-> left_son) { try (node_fold_constants (node-> left_son, folded_anything)); }
    if (node->right_son) { try (node_fold_constants (node->right_son, folded_anything)); }


    switch (node->atom_type) {

        case DAT_OPERATION: try (operation_fold_constants (node, folded_anything)); break;
        case DAT_CONST:     break;
        case DAT_VARIABLE:  break;

        default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }


    return SUCCESS;
}


//--------------------------------------------------
#define MY_VAL    node->element.value.val_double
#define LEFT_VAL  node->left_son->element.value.val_double
#define RIGHT_VAL node->right_son->element.value.val_double
#define I_FOLDED  *folded_anything = true
//--------------------------------------------------


Return_code  operation_fold_constants  (Node* node, bool* folded_anything) {

    if (!node || (node->atom_type != DAT_OPERATION) || !folded_anything) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (node->left_son->atom_type != DAT_CONST || node->right_son->atom_type != DAT_CONST) { return SUCCESS; }


    if (!isfinite (LEFT_VAL) || !isfinite (RIGHT_VAL)) return BAD_ARGS;



    switch (node->element.value.val_operation_code) {

        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return BAD_ARGS;

        case DOC_ADD:

            MY_VAL = LEFT_VAL + RIGHT_VAL; I_FOLDED;
            break;

        case DOC_SUB:

            MY_VAL = LEFT_VAL - RIGHT_VAL; I_FOLDED;
            break;

        case DOC_MULT:

            MY_VAL = LEFT_VAL * RIGHT_VAL; I_FOLDED;
            break;

        case DOC_DIV:

            if (double_equal (RIGHT_VAL, 0)) return BAD_ARGS;
            MY_VAL = LEFT_VAL / RIGHT_VAL; I_FOLDED;
            break;

        case DOC_POW:

            if (double_equal (LEFT_VAL, 0) && RIGHT_VAL < 0)                                   return BAD_ARGS;
            if (LEFT_VAL < 0 && !double_equal ( (RIGHT_VAL - (double) (size_t) RIGHT_VAL), 0)) return BAD_ARGS; //целая ли степень

            MY_VAL = pow (LEFT_VAL, RIGHT_VAL);

            break;

        case DOC_LN:  return SUCCESS;
        case DOC_SIN: return SUCCESS;
        case DOC_COS: return SUCCESS;
        case DOC_TG:  return SUCCESS;
        case DOC_CTG: return SUCCESS;


        default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }


    node->atom_type = DAT_CONST;


    I_FOLDED;


    node_dtor (node-> left_son);
    node_dtor (node->right_son);
    node->right_son = nullptr;
    node-> left_son = nullptr;


    return SUCCESS;
}


//--------------------------------------------------
#undef MY_VAL
#undef LEFT_VAL
#undef RIGHT_VAL
#undef I_FOLDED
//--------------------------------------------------


Node*  copy_node  (Node* node) {

    if (!node) { LOG_ERROR (BAD_ARGS); return nullptr; }


    Node* new_node = (Node*) calloc (NODE_SIZE, 1);


    switch (node->atom_type) {

        case DAT_OPERATION: new_node = create_node (DAT_OPERATION, node->element.value.val_operation_code); break;
        case DAT_CONST:     new_node = create_node (DAT_CONST,     node->element.value.val_double);         break;
        case DAT_VARIABLE:  new_node = create_node (DAT_VARIABLE,  node->element.value.var_str);            break;

        default: LOG_ERROR (BAD_ARGS); return nullptr;
    }


    if (node-> left_son) { new_node-> left_son = copy_node (node-> left_son); }
    if (node->right_son) { new_node->right_son = copy_node (node->right_son); }


    return new_node;
}


Return_code  tree_fold_neutral  (Tree* tree, bool* folded_anything) {

    if (!tree || !folded_anything) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (node_fold_neutral (tree->root, folded_anything));


    return SUCCESS;
}


Return_code  node_fold_neutral  (Node* node, bool* folded_anything) {

    if (!node || !folded_anything) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (node-> left_son) { try (node_fold_neutral (node-> left_son, folded_anything)); }
    if (node->right_son) { try (node_fold_neutral (node->right_son, folded_anything)); }


    switch (node->atom_type) {

        case DAT_OPERATION: try (operation_fold_neutral (node, folded_anything)); break;
        case DAT_CONST:     break;
        case DAT_VARIABLE:  break;

        default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }


    return SUCCESS;
}


//--------------------------------------------------
#define LEFT  node-> left_son
#define RIGHT node->right_son

#define OPERATION_CODE element.value.val_operation_code
#define CONST          element.value.val_double
#define VARIABLE       element.value.val_str

#define PROMOTE_LEFT\
    _node_dtor (RIGHT);\
    node_realloc (LEFT, node);\
    *folded_anything = true;\
    return SUCCESS;

#define PROMOTE_RIGHT\
    _node_dtor (LEFT);\
    node_realloc (RIGHT, node);\
    *folded_anything = true;\
    return SUCCESS;

#define NODE_TO(x)\
    _node_dtor (LEFT);\
    _node_dtor (RIGHT);\
\
    node->atom_type = DAT_CONST;\
    node->CONST     = x;\
    node-> left_son = nullptr;\
    node->right_son = nullptr;\
    *folded_anything = true;\
    return SUCCESS;

#define  LEFT_IS(x) (LEFT-> atom_type == DAT_CONST && double_equal (LEFT-> CONST, x) )
#define RIGHT_IS(x) (RIGHT->atom_type == DAT_CONST && double_equal (RIGHT->CONST, x) )
//--------------------------------------------------


Return_code  operation_fold_neutral  (Node* node, bool* folded_anything) {

    if (!node || (node->atom_type != DAT_OPERATION) || !folded_anything) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    switch (node->OPERATION_CODE) {

        case DOC_ADD:

            if ( LEFT_IS (0) ) { PROMOTE_RIGHT }
            if (RIGHT_IS (0) ) { PROMOTE_LEFT }

            return SUCCESS;

        case DOC_SUB:

            if (RIGHT_IS (0) ) { PROMOTE_LEFT }

            return SUCCESS;

        case DOC_MULT:

            if ( LEFT_IS (1) ) { PROMOTE_RIGHT }
            if (RIGHT_IS (1) ) { PROMOTE_LEFT }

            if ( LEFT_IS (0) ) { NODE_TO (0) }
            if (RIGHT_IS (0) ) { NODE_TO (0) }

            return SUCCESS;

        case DOC_DIV:

            if (RIGHT_IS (1) ) { PROMOTE_LEFT }

            if (LEFT_IS  (0) ) { NODE_TO (0) }

            if (RIGHT_IS (0) ) { return BAD_ARGS; }

            return SUCCESS;

        case DOC_POW:

            if ( LEFT_IS (0) ) { NODE_TO (0) }
            if ( LEFT_IS (1) ) { NODE_TO (1) }
            if (RIGHT_IS (0) ) { NODE_TO (1) }
            if (RIGHT_IS (1) ) { PROMOTE_LEFT }

            return SUCCESS;

        case DOC_LN:

            if (RIGHT->atom_type == DAT_CONST && (RIGHT->CONST < 0 || double_equal (RIGHT->CONST, 0)) ) return BAD_ARGS;

            if (RIGHT_IS (1)) { NODE_TO (0) }

            return SUCCESS;

        case DOC_SIN:

            if (RIGHT_IS (0)) { NODE_TO (0) }

            return SUCCESS;

        case DOC_COS:

            if (RIGHT_IS (0)) { NODE_TO (1) }

            return SUCCESS;

        case DOC_TG:

            if (RIGHT_IS (0)) { NODE_TO (0) }

            return SUCCESS;

        case DOC_CTG:

            if (RIGHT_IS (0)) return BAD_ARGS;

            return SUCCESS;


        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        default:          LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }
}


//--------------------------------------------------
#undef LEFT
#undef RIGHT
#undef OPERATION_CODE
#undef CONST
#undef VARIABLE
#undef PROMOTE_LEFT
#undef PROMOTE_RIGHT
#undef NODE_TO_0
//--------------------------------------------------


Return_code  tree_fold  (Tree* tree, bool silent, FILE* file, int precision) {

    if (!tree)            { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (!silent && !file) { LOG_MESSAGE ("can't write into nullptr file! (specify write file if you didn't)"); silent = true; }



    if (!silent) {

        tex_write_tree (file, tree, precision);
        fprintf (file, "$$\n");
    } 



    bool folded_anything_now = true;

    while (folded_anything_now) {

        folded_anything_now = false;

        try (tree_fold_constants (tree, &folded_anything_now));
        try (tree_fold_neutral   (tree, &folded_anything_now));


        if (!silent && folded_anything_now) {

            fprintf (file, "$$ = ");
            tex_write_tree (file, tree, precision);
            fprintf (file, "$$\n");
        }
    }


    return SUCCESS;
}


Return_code  _dfr_buffer_read  (Dfr_buffer* dfr_buffer, char* str) {

    if (!dfr_buffer || !str) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    dfr_buffer->buffer = (Buffer_node*) calloc (BUFFER_NODE_SIZE * strlen (str), 1);



    char*  str_max  = str + strlen (str);
    size_t cur_node = 0;

    skipspaces (&str);

    while (str < str_max) {

        read_buffer_operation (&str, &dfr_buffer->buffer [cur_node]);
        skipspaces (&str);
        cur_node += 1;
    }

    dfr_buffer->len = cur_node;


    return SUCCESS;
}


Return_code  read_buffer_operation  (char** str_ptr, Buffer_node* buffer_node) {

    if (!str_ptr || !*str_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    switch (**str_ptr) {

        case '(': *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_OPEN_BRACKET };    return SUCCESS;
        case ')': *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_CLOSING_BRACKET }; return SUCCESS;
        case '+': *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_ADD };             return SUCCESS;
        case '-': *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_SUB };             return SUCCESS;
        case '/': *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_DIV };             return SUCCESS;
        case '*':

            *str_ptr += 1;
            if (**str_ptr == '*') { *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_POW };  } //pow
            else                  {                *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_MULT }; } //mult

            return SUCCESS;

        case 's': //sin

            if (*(*str_ptr + 1) == 'i' && *(*str_ptr + 2) == 'n' && !is_variable_mid (*(*str_ptr + 3)) ) {

                *str_ptr += 3;
                *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_SIN };
                return SUCCESS;
            }

            break;

        case 'c': //cos ctg

            if (*(*str_ptr + 1) == 'o' && *(*str_ptr + 2) == 's' && !is_variable_mid (*(*str_ptr + 3)) ) {

                *str_ptr += 3;
                *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_COS };
                return SUCCESS;
            }

            if (*(*str_ptr + 1) == 't' && *(*str_ptr + 2) == 'g' && !is_variable_mid (*(*str_ptr + 3)) ) {

                *str_ptr += 3;
                *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_CTG };
                return SUCCESS;
            }

            break;

        case 't': //tg

            if (*(*str_ptr + 1) == 'g' && !is_variable_mid (*(*str_ptr + 2)) ) {

                *str_ptr += 2;
                *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_TG };
                return SUCCESS;
            }

            break;

        case 'l': //tg

            if (*(*str_ptr + 1) == 'n' && !is_variable_mid (*(*str_ptr + 2)) ) {

                *str_ptr += 2;
                *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_LN };
                return SUCCESS;
            }

            break;


        default: break;
    }



    return read_buffer_variable (str_ptr, buffer_node);
}


Return_code  read_buffer_variable  (char** str_ptr, Buffer_node* buffer_node) {

    if (!str_ptr || !*str_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!is_variable_start (**str_ptr)) return read_buffer_number (str_ptr, buffer_node);


    char* variable = (char*) calloc (MAX_VARIABLE_LEN + 1, 1);
    variable [0] = **str_ptr; *str_ptr += 1;


    size_t variable_index = 1;
    while (is_variable_mid (**str_ptr)) {

        variable [variable_index] = **str_ptr;

        variable_index += 1;
        *str_ptr       += 1;
    }


    variable [variable_index] = '\0';


    *buffer_node = { DAT_VARIABLE, .val_str = variable };


    return SUCCESS;
}


Return_code  read_buffer_number  (char** str_ptr, Buffer_node* buffer_node) {

    if (!str_ptr || !*str_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    double value = 0;

    const char* old_string = *str_ptr;

    while (**str_ptr >= '0' && **str_ptr <= '9') {

        value = (value * 10) + (**str_ptr - '0');
        *str_ptr += 1;
    }

    assert (*str_ptr > old_string);


    *buffer_node = { DAT_CONST, .val_double = value };


    return SUCCESS;
}


Return_code  skipspaces  (char** str_ptr) {

    if (!str_ptr || !*str_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    while (isspace (**str_ptr) || (**str_ptr == '/')) {

        if (**str_ptr == '/') {

            if (*(*str_ptr + 1) == '/') {

                while (**str_ptr != '\n' && *(*str_ptr + 1) != '\0') {

                    *str_ptr += 1;
                }

                *str_ptr += 1;
                continue;
            }

            if (*(*str_ptr + 1) == '*') {

                while ( (*(*str_ptr - 1) != '*' || **str_ptr != '/') 
                      && *(*str_ptr + 1) != '\0') {


                    *str_ptr += 1;
                }

                *str_ptr += 1;
                continue;
            }

            break;
        }


        *str_ptr += 1;
    }


    return SUCCESS;
}


Return_code  dfr_buffer_ctor  (Dfr_buffer* dfr_buffer) {

    if (!dfr_buffer) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    dfr_buffer->buffer = nullptr;
    dfr_buffer->len    = 0;


    return SUCCESS; 
}


Return_code  dfr_buffer_dtor  (Dfr_buffer* dfr_buffer) {

    if (!dfr_buffer) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    for (size_t i = 0; i < dfr_buffer->len; i++) {

        if (dfr_buffer->buffer [i].atom_type == DAT_VARIABLE) {

            free (dfr_buffer->buffer [i].val_str);
        }
    }



    free (dfr_buffer->buffer);


    dfr_buffer->buffer = nullptr;


    return SUCCESS;
}


Return_code  tree_evaluate  (Tree* tree, Tree* answer, const char* variable, double value, bool silent, FILE* file, int precision) {

    if (!tree || !answer || !variable || !isfinite (value)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    if (!silent && !file) { LOG_MESSAGE ("can't write into nullptr file! (specify write file if you didn't)"); silent = true; }


    if (answer->root) node_dtor (answer->root);


    answer->root = copy_node (tree->root);
    tree_substitute_variable (answer, variable, value);


    try (tree_fold (answer, silent, file, precision));


    return SUCCESS;
}


Return_code  tree_substitute_variable  (Tree* tree, const char* variable, double value) {

    if (!tree || ! variable || !isfinite (value)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!tree->root) return SUCCESS;


    node_substitute_variable (tree->root, variable, value);


    return SUCCESS;
}


Return_code  node_substitute_variable  (Node* node, const char* variable, double value) {

    if (!node || ! variable || !isfinite (value)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (node->atom_type == DAT_VARIABLE && !strcmp (node->element.value.var_str, variable) ) {

        node->atom_type = DAT_CONST;
        node->element.value.val_double = value;
    }


    if (node-> left_son) node_substitute_variable (node-> left_son, variable, value);
    if (node->right_son) node_substitute_variable (node->right_son, variable, value);


    return SUCCESS;
}


Return_code  dfr_calculate_taylor  (Dfr* dfr, const char* variable, double point, size_t depth, bool silent, FILE* file, int precision) {

    if (!dfr || !variable ||!isfinite (point) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    if (!dfr->derivative_trees_array [0]) {

        LOG_MESSAGE ("can't calculate taylor without function tree!");
        LOG_ERROR (BAD_ARGS);
        return BAD_ARGS;
    }

    if (!silent && !file) { LOG_MESSAGE ("can't write into nullptr file! (specify write file if you didn't)"); silent = true; }



    if (!silent) tex_write_taylor_introduction (file, depth, variable, point, precision);


    for (size_t i = 0; i <= depth; i++) {

        try (dfr_ensure_derivative (dfr, variable, i));



        if (!silent) tex_write_evaluation_introduction (file, i, point, precision);


        Tree* evaluation_tree = (Tree*) calloc (TREE_SIZE, 1); TREE_CTOR (evaluation_tree);
        try (tree_evaluate (dfr->derivative_trees_array [i], evaluation_tree, variable, point, silent, file, precision));


        if (!silent) tex_write_evaluation_ending (file, i, point, evaluation_tree, precision);



        Tree* cur_taylor_tree = (Tree*) calloc (TREE_SIZE, 1);
        TREE_CTOR (cur_taylor_tree);
        tree_kill_root (cur_taylor_tree);


        cur_taylor_tree->root = create_node (DAT_OPERATION, DOC_DIV);


        cur_taylor_tree->root->left_son = evaluation_tree->root;
        tree_kill_tree (evaluation_tree);


        cur_taylor_tree->root->right_son = create_node (DAT_CONST, factorial (i));



        try (tree_fold (cur_taylor_tree));


        dfr->taylor [i] = cur_taylor_tree;
    }


    if (!silent) tex_write_taylor_ending (file, dfr, depth, variable, point, precision);


    return SUCCESS;
}


Return_code  dfr_ensure_derivative  (Dfr* dfr, const char* variable, size_t n, bool silent, FILE* file) {

    if (!dfr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    if (!silent && !file) { LOG_MESSAGE ("can't write into nullptr file! (specify write file if you didn't)"); silent = true; }



    if (dfr->derivative_trees_array [n]) return SUCCESS;


    if (n == 0) {

        LOG_MESSAGE ("can't fill in 0'th derivative (aka user function tree)");
        LOG_ERROR (BAD_ARGS);
        return BAD_ARGS;
    }


    dfr_ensure_derivative (dfr, variable, n - 1, silent, file);


    dfr_calculate_derivative (dfr, variable, n, silent, file);


    return SUCCESS;
}


double  factorial  (size_t n) {

    if (n == 0) return 1;


    return (double) n * factorial (n - 1);
}


Return_code  tex_write_tree  (FILE* file, Tree* tree, int precision) {

    if (!tree || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Tree_substitution substitutions = {};
    tree_substitution_ctor (&substitutions);
    //fprintf (file, " |debug-1| ");


    tree_substitute (tree, &substitutions);// fprintf (file, " |debug0| ");
    printf ("TOTAL SUBS: %zd\n", substitutions.num_substitutions);


    tex_write_node (file, tree->root, &substitutions, precision);// fprintf (file, " |debug1| ");


    tex_write_substitutions_decoding (file, &substitutions, precision);// fprintf (file, " |debug2| ");


    tree_substitution_dtor (&substitutions);


    return SUCCESS;
}


Return_code  tex_write_node  (FILE* file, Node* node, Tree_substitution* substitutions, int precision) {

    if (!node || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if (substitutions) {

        Node_substitution* substitution = get_tex_substitution (substitutions, node);
        if (substitution) {

            tex_write_substitution (file, substitution);
            return SUCCESS;
        }
    }




    switch (node->atom_type) {

        case DAT_CONST:     tex_write_const     (file, node,                precision); return SUCCESS;
        case DAT_VARIABLE:  fprintf (file, "%s", node->element.value.var_str);          return SUCCESS;
        case DAT_OPERATION: tex_write_operation (file, node, substitutions, precision); return SUCCESS;

        default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }
}


//--------------------------------------------------
#define WRITE_LEFT\
        tex_write_check_open_bracket    (file, node->left_son, node);\
        tex_write_node                  (file, node->left_son, substitutions, precision);\
        tex_write_check_closing_bracket (file, node->left_son, node);

#define WRITE_RIGHT\
        tex_write_check_open_bracket    (file, node, node->right_son);\
        tex_write_node                  (file, node->right_son, substitutions, precision);\
        tex_write_check_closing_bracket (file, node, node->right_son);
//--------------------------------------------------


Return_code  tex_write_operation  (FILE* file, Node* node, Tree_substitution* substitutions, int precision) {

    if (!node || !file || node->atom_type != DAT_OPERATION) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    switch (node->element.value.val_operation_code) {

        case DOC_ADD:

            WRITE_LEFT;
            fprintf (file, " + ");
            WRITE_RIGHT;
            return SUCCESS;

        case DOC_SUB:

            if (is_unary_minus (node)) {

                fprintf (file, "-");
                tex_write_node (file, node->right_son, substitutions, precision);
                //fprintf (file, "");
                return SUCCESS;
            }


            WRITE_LEFT;
            fprintf (file, " - ");
            WRITE_RIGHT;
            return SUCCESS;

        case DOC_MULT:

            WRITE_LEFT;
            fprintf (file, " \\cdot ");
            WRITE_RIGHT;
            return SUCCESS;

        case DOC_DIV:

            fprintf (file, "\\frac { ");
            tex_write_node (file, node-> left_son, substitutions, precision);
            fprintf (file, " } { ");
            tex_write_node (file, node->right_son, substitutions, precision);
            fprintf (file, " }");
            return SUCCESS;

        case DOC_POW:

            WRITE_LEFT;
            fprintf (file, "^{");
            tex_write_node (file, node->right_son, substitutions, precision);
            fprintf (file, " }");
            return SUCCESS;

        case DOC_LN:

            fprintf (file, "\\ln ");
            WRITE_RIGHT;
            return SUCCESS;

        case DOC_SIN:

            fprintf (file, "\\sin ");
            WRITE_RIGHT;
            return SUCCESS;

        case DOC_COS:

            fprintf (file, "\\cos ");
            WRITE_RIGHT;
            return SUCCESS;

        case DOC_TG:

            fprintf (file, "\\tan ");
            WRITE_RIGHT;
            return SUCCESS;

        case DOC_CTG:

            fprintf (file, "\\cot ");
            WRITE_RIGHT;
            return SUCCESS;

        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return BAD_ARGS; 
        default:          LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }
}


//--------------------------------------------------
#undef WRITE_LEFT
#undef WRITE_RIGHT
//--------------------------------------------------


Return_code  tex_write_const  (FILE* file, Node* node, int precision) {

    if (!file || !node || (node->atom_type != DAT_CONST)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (node->element.value.val_double < 0) fprintf (file, "(");


    fprintf (file, "%.*lf", precision, node->element.value.val_double);


    if (node->element.value.val_double < 0) fprintf (file, ")");


    return SUCCESS;
}


Return_code  tex_write_check_open_bracket  (FILE* file, Node* left, Node* right) {

    if (!left || !right || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (left->atom_type != DAT_OPERATION || right->atom_type != DAT_OPERATION) return SUCCESS;


    if (!are_associative (left->element.value.val_operation_code, right->element.value.val_operation_code)) fprintf (file, "( ");


    return SUCCESS;
}


Return_code  tex_write_check_closing_bracket  (FILE* file, Node* left, Node* right) {

    if (!left || !right || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (left->atom_type != DAT_OPERATION || right->atom_type != DAT_OPERATION) return SUCCESS;


    if (!are_associative (left->element.value.val_operation_code, right->element.value.val_operation_code)) fprintf (file, " )");


    return SUCCESS;
}


Return_code  tex_generate_output  (Dfr* dfr, const char* variable, double taylor_point, size_t depth, double tangent_point, int precision, const char* file_name) {

    if (!dfr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!dfr->user_function_tree) {

        LOG_MESSAGE ("can't generate output not knowing function");
        LOG_ERROR (BAD_ARGS);
        return BAD_ARGS;
    }


    if (precision > 6) precision = 6;




    FILE* file = fopen (file_name, "w");
    if (!file) { LOG_ERROR (FILE_ERR); return FILE_ERR; }


    try (tex_write_preamble      (file));
    try (tex_write_introduction  (file, dfr, variable,                      precision));
    try (tex_write_user_function (file, dfr, variable,                      precision));
    try (tex_write_derivative    (file, dfr, variable,                      precision));
    try (tex_write_taylor        (file, dfr, variable, taylor_point, depth, precision));
    try (tex_write_graph         (file, dfr, variable, tangent_point,       precision));
    try (tex_write_end           (file));


    return SUCCESS;
}


Return_code  tex_write_preamble  (FILE* file) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\\documentclass                                                    {article}\n");
    fprintf (file, "\\usepackage                                                       {mathtools}\n");
    fprintf (file, "\\usepackage [utf8]                                                {inputenc}\n");
    fprintf (file, "\\usepackage [top = 1 cm, bottom = 1 cm, left = 1cm, right = 1 cm] {geometry}\n");
    fprintf (file, "\\usepackage                                                       {graphicx}\n");
    fprintf (file, "\\usepackage                                                       {amsfonts}\n");
    fprintf (file, "\\usepackage [T2A]                                                 {fontenc}\n");
    fprintf (file, "\\usepackage                                                       {amssymb}\n");
    fprintf (file, "\n");
    fprintf (file, "\n");
    fprintf (file, "\\title  {Differentiation of elementary functions of a real argument research}\n");
    fprintf (file, "\\author {Grigory Grigorievich}\n");
    fprintf (file, "\\date   {December 2022}\n");
    fprintf (file, "\n");
    fprintf (file, "\\begin {document} \\Large\n");
    fprintf (file, "\n");
    fprintf (file, "\\maketitle\n");
    fprintf (file, "\n");


    return SUCCESS;
}


Return_code  tex_write_end  (FILE* file) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\n");
    fprintf (file, "\n");
    fprintf (file, "\\end{document}");


    return SUCCESS;
}


const char*  tex_get_phrase  (void) {

    static bool first_time_enter = true;

    if (first_time_enter) srand ( (unsigned int) time (nullptr));

    first_time_enter = false;


    switch (rand () % 7) {

        case 0: return "очевидно, что ";
        case 1: return "легко видеть, что ";
        case 2: return "внимательный читалель заметит, что ";
        case 3: return "доказательство следующего утверждения остается в качестве упражнения читателю: ";
        case 4: return "заметим, что ";
        case 5: return "воспользуемся тем, что ";
        case 6: return "по методу Султанова, ";

        default: LOG_MESSAGE ("BAD RANDOM GENERATED"); return nullptr;
    }
}


Return_code  tex_write_introduction  (FILE* file, Dfr* dfr, const char* variable, int precision) {

    if (!dfr || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\\section {Введение}\n");
    fprintf (file, "Сегодня мы обратим внимание на дифференцирование следующего представителя класса элементарных функций действительного аргумента: ");


    fprintf (file, "$$");
    fprintf (file, "%s(%s) = ", USER_FUNCTION_NAME, variable);
    tex_write_tree (file, dfr->user_function_tree, precision);
    fprintf (file, "$$\n\n");


    return SUCCESS;
}


Return_code  tex_write_derivative  (FILE* file, Dfr* dfr, const char* variable, int precision) {

    if (!dfr || !variable || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\\section {Поиск производной}\n");


    dfr_calculate_derivative (dfr, variable, 1, false, file, precision);


    return SUCCESS;
}


Tree*  copy_tree  (Tree* tree) {

    if (!tree) { LOG_ERROR (BAD_ARGS); return nullptr; }


    Tree* new_tree = (Tree*) calloc (TREE_SIZE, 1);

    TREE_CTOR (new_tree);
    tree_kill_root (new_tree);
    new_tree->root = copy_node (tree->root);


    return new_tree;
}


Return_code  tex_write_function_name  (FILE* file, size_t derivative_num) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "%s", USER_FUNCTION_NAME);

    if (derivative_num == 0) { return SUCCESS; }
    if (derivative_num == 1) { fprintf (file, "'"); }
    else {                     fprintf (file, "^{(%zd)}", derivative_num); }


    return SUCCESS;
}


Return_code  tex_write_taylor  (FILE* file, Dfr* dfr, const char* variable, double point, size_t depth, int precision) {

    if (!file || !dfr || !variable || !isfinite (point) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (dfr_calculate_taylor (dfr, variable, point, depth, false, file, precision));


    return SUCCESS;
}


Return_code  tex_write_evaluation_introduction  (FILE* file, size_t derivative_num, double value, int precision) {

    if (!file || !isfinite (value)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\\subsection {давайте найдем $");
    tex_write_function_name (file, derivative_num);
    fprintf (file, "(%.*lf)$}\n", precision, value);

    fprintf (file, "\\begin {flushleft}\n");
    fprintf (file, "%s\n", tex_get_phrase ());
    fprintf (file, "\\end {flushleft}\n");

    fprintf (file, "\n$$");
    tex_write_function_name (file, derivative_num);
    fprintf (file, "(%.*lf) = ", precision, value);


    return SUCCESS;
}


Return_code  tex_write_evaluation_ending  (FILE* file, size_t derivative_num, double value, Tree* answer, int precision) {

    if (!file || !isfinite (value) || !answer) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\n");
    fprintf (file, "\\begin {flushleft}\n");
    fprintf (file, "итак, \n");
    fprintf (file, "\\end {flushleft}\n\n");
    fprintf (file, "$$");
    tex_write_function_name (file, derivative_num);
    fprintf (file, "(%.*lf) = ", precision, value);
    tex_write_tree (file, answer, precision);
    fprintf (file, "$$\n\n");


    return SUCCESS;
}


Return_code  tex_write_taylor_introduction  (FILE* file, size_t depth, const char* variable, double point, int precision) {

    if (!file || !variable || !isfinite (point) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\\section {Разложение в ряд тейлора}\n");
    fprintf (file, "\\begin {flushleft}\n");
    fprintf (file, "давайте найдем разложение в ряд тейлора функции $");
    tex_write_function_name (file, 0);
    fprintf (file, "(%s)$ в точке %.*lf до o($(%s - %.*lf)^%zd$)\n", variable, precision, point, variable, precision, point, depth);
    fprintf (file, "\\end {flushleft}\n\n");


    return SUCCESS;
}


Return_code  tex_write_taylor_ending  (FILE* file, Dfr* dfr, size_t depth, const char* variable, double point, int precision) {

    if (!dfr || !file || !variable || !isfinite (point) || !dfr->taylor) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\\begin {flushleft}\n");
    fprintf (file, "разложение функции %s(%s) в ряд тейлора в точке %.*lf: \n", USER_FUNCTION_NAME, variable, precision, point);
    fprintf (file, "\\end {flushleft}\n\n");


    fprintf (file, "$$");
    tex_write_tree (file, dfr->taylor [0], precision);
    fprintf (file, "$$");

    for (size_t i = 1; i <= depth; i++) {

        fprintf (file, "$$ + ");
        try (tex_write_tree (file, dfr->taylor [i], precision));
        fprintf (file, " \\cdot (%s - %.*lf)^%zd$$", variable, precision, point, i);

    }


    fprintf (file, "$$ + o((%s - %.*lf)^%zd)$$", variable, precision, point, depth);


    fprintf (file, "\n\n");


    return SUCCESS;
}


Return_code  tex_write_derivative_introduction  (FILE* file, const char* variable, size_t derivative_num) {

    if (!file || !variable) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\\subsection {давайте найдем $");
    tex_write_function_name (file, derivative_num);
    fprintf (file, "(%s)$}\n", variable);

    fprintf (file, "\\begin {flushleft}\n");
    fprintf (file, "%s\n", tex_get_phrase ());
    fprintf (file, "\\end {flushleft}\n\n");
    fprintf (file, "\n$$");
    tex_write_function_name (file, derivative_num);
    fprintf (file, "(%s) = ", variable);


    return SUCCESS;
}


Return_code  tex_write_derivative_ending  (FILE* file, Dfr* dfr, const char* variable, size_t derivative_num, int precision) {

    if (!dfr || !file || !variable || !dfr->derivative_trees_array [derivative_num]) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\n");
    fprintf (file, "\\begin {flushleft}\n");
    fprintf (file, "итак, \n");
    fprintf (file, "\\end {flushleft}\n\n");
    fprintf (file, "$$");
    tex_write_function_name (file, derivative_num);
    fprintf (file, "(%s) = ", variable);
    tex_write_tree (file, dfr->derivative_trees_array [derivative_num], precision);
    fprintf (file, "$$\n\n");


    return SUCCESS;
}


Return_code  tex_write_user_function  (FILE* file, Dfr* dfr, const char* variable, int precision) {

    if (!file || !dfr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!dfr->user_function_tree) { LOG_MESSAGE ("please enter user function first!"); return BAD_ARGS; }


    fprintf (file, "\\section {Упрощение функции}\n");


    dfr->derivative_trees_array [0] = copy_tree (dfr->user_function_tree);


    tex_write_user_function_introduction (file, variable);
    try (tree_fold                       (dfr->derivative_trees_array [0], false, file, precision));
    tex_write_user_function_ending       (file, dfr, variable, precision);



    return SUCCESS;
}


Return_code  tex_write_user_function_introduction (FILE* file, const char* variable) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "%s", tex_get_phrase ());
    fprintf (file, "$$");
    tex_write_function_name (file, 0);
    fprintf (file, "(%s) = ", variable);


    return SUCCESS;
} 


Return_code  tex_write_user_function_ending  (FILE* file, Dfr* dfr, const char* variable, int precision) {

    if (!file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "\nитак, \n$$");
    tex_write_function_name (file, 0);
    fprintf (file, "(%s) = ", variable);
    tex_write_tree (file, dfr->derivative_trees_array [0], precision);
    fprintf (file, "$$\n\n");


    return SUCCESS;
}


Return_code  node_get_tex_len  (Node* node, double* len_ptr, double len_coefficient, Tree_substitution* substitution_list) {

    if (!node || !len_ptr || !isfinite (*len_ptr) || !isfinite (len_coefficient) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if (substitution_list) {

        double substitution_len = get_tex_substitution_len (substitution_list, node);

        if (!!isfinite (substitution_len)) {

            *len_ptr += len_coefficient * substitution_len;
            return SUCCESS;
        }
    }



    switch (node->atom_type) {

        case DAT_CONST:     double_get_tex_len    (node->element.value.val_double, len_ptr, len_coefficient);                    return SUCCESS;
        case DAT_VARIABLE:  variable_get_tex_len  (node->element.value.var_str,    len_ptr, len_coefficient);                    return SUCCESS;
        case DAT_OPERATION: operation_get_tex_len (node,                           len_ptr, len_coefficient, substitution_list); return SUCCESS;

        default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }
}


Return_code  double_get_tex_len  (double value, double* len_ptr, double len_coefficient) {

    if (!len_ptr || !isfinite (*len_ptr) || !isfinite (len_coefficient) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (value < 0) { *len_ptr += len_coefficient * 1; value *= -1; } //for -


    if (value < 2) value += 2; //for correct log (value) work


    *len_ptr += len_coefficient * ceil (log10 (value)); //for all the digits


    *len_ptr += len_coefficient * 2; //for .0 at the end


    return SUCCESS;
}


size_t  size_t_get_tex_len  (size_t value) {

    if (value < 2) value = 2;


    return (size_t) ceil (log10 (ceil (value)));
}


Return_code  variable_get_tex_len  (const char* variable, double* len_ptr, double len_coefficient) {

    if (!variable || !len_ptr || !isfinite (*len_ptr) || !isfinite (len_coefficient) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    *len_ptr += len_coefficient * (double) strlen (variable);


    return SUCCESS;
}


Return_code  operation_get_tex_len  (Node* node, double* len_ptr, double len_coefficient, Tree_substitution* substitutions) {

    if (!node || (node->atom_type != DAT_OPERATION) || !isfinite (*len_ptr) || !isfinite (len_coefficient)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    switch (node->element.value.val_operation_code) {

        case DOC_ADD:

            try (node_get_tex_len (node-> left_son, len_ptr, len_coefficient, substitutions));
            *len_ptr += len_coefficient * 1; //for +
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions));
            return SUCCESS;

        case DOC_SUB:

            try (node_get_tex_len (node-> left_son, len_ptr, len_coefficient, substitutions));
            *len_ptr += len_coefficient * 1; //for -
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions));
            return SUCCESS;

        case DOC_MULT:

            try (node_get_tex_len (node-> left_son, len_ptr, len_coefficient, substitutions));
            *len_ptr += len_coefficient * 1; //for \cdot
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions));
            return SUCCESS;

        case DOC_DIV: {

            double son_len_coefficient = 1;
            double len_left            = 0;
            double len_right           = 0;

            try (node_get_tex_len (node-> left_son, &len_left,  son_len_coefficient, substitutions));
            try (node_get_tex_len (node->right_son, &len_right, son_len_coefficient, substitutions));

            *len_ptr += len_coefficient * fmax (len_left, len_right);


            return SUCCESS;
        }

        case DOC_POW:

            try (node_get_tex_len (node-> left_son, len_ptr, len_coefficient, substitutions));
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions)); //with coef
            return SUCCESS;

        case DOC_LN:

            *len_ptr += len_coefficient * 2; //for ln
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions));
            return SUCCESS;

        case DOC_SIN:

            *len_ptr += len_coefficient * 3; //for sin
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions));
            return SUCCESS;

        case DOC_COS:

            *len_ptr += len_coefficient * 3; //for cos
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions));
            return SUCCESS;

        case DOC_TG:

            *len_ptr += len_coefficient * 2; //for tg
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions));
            return SUCCESS;

        case DOC_CTG:

            *len_ptr += len_coefficient * 3; //for ctg
            try (node_get_tex_len (node->right_son, len_ptr, len_coefficient, substitutions));
            return SUCCESS;


        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        default:          LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }
}


Return_code  tree_substitute  (Tree* tree, Tree_substitution* tree_substitution) {

    if (!tree) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    printf ("NEW_SUBSTITUITON BEGINS\n");
    node_substitute (tree->root, tree_substitution, 1);


    return SUCCESS;
}


Return_code  tree_substitution_ctor  (Tree_substitution* tree_substitution) {

    if (!tree_substitution) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    tree_substitution->substitution_list = (Node_substitution*) calloc (MAX_SUBSTITUTIONS * NODE_SUBSTITUTION_SIZE, 1);
    tree_substitution->num_substitutions = 0;

    for (size_t i = 0; i < MAX_SUBSTITUTIONS; i++) {

        tree_substitution->substitution_list [i] = {nullptr, i};
    }


    return SUCCESS;
}


Return_code  tree_substitution_dtor  (Tree_substitution* tree_substitution)  {

    if (!tree_substitution) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    free (tree_substitution->substitution_list);


    return SUCCESS;
}


double  get_operation_tex_len  (Operation_code operation_code, double len_coefficient) {

    switch (operation_code) {

        case DOC_ADD:  return len_coefficient * 1;
        case DOC_SUB:  return len_coefficient * 1;
        case DOC_MULT: return len_coefficient * 1;
        case DOC_DIV:  return 0;
        case DOC_POW:  return 0;
        case DOC_LN:   return len_coefficient * 2;
        case DOC_SIN:  return len_coefficient * 3;
        case DOC_COS:  return len_coefficient * 3;
        case DOC_TG:   return len_coefficient * 2;
        case DOC_CTG:  return len_coefficient * 3;

        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return NAN;
        default:          LOG_ERROR (BAD_ARGS); return NAN;
    }
}


Return_code  node_substitute  (Node* node, Tree_substitution* tree_substitution, double cur_len_coefficient) {

    if (!node || !tree_substitution) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (node->atom_type != DAT_OPERATION) return SUCCESS; //can't substitute if node has no sons!

    //printf ("node sub\n");

    node_substitute_left  (node, tree_substitution, cur_len_coefficient);
    node_substitute_right (node, tree_substitution, cur_len_coefficient);


    return SUCCESS;
}


Return_code  node_substitute_left  (Node* node, Tree_substitution* tree_substitution, double cur_len_coefficient) {

    if (!node || !tree_substitution) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if ( (node->atom_type != DAT_OPERATION) || !node->left_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    double len_left = 0;
    node_get_tex_len (node->left_son, &len_left, cur_len_coefficient, tree_substitution);


    if (len_left < (MAX_TEX_LEN - get_operation_tex_len (node->element.value.val_operation_code, cur_len_coefficient)) / 3) return SUCCESS;


    node_substitute (node->left_son, tree_substitution, cur_len_coefficient);


    len_left = 0;
    node_get_tex_len (node->left_son, &len_left, cur_len_coefficient, tree_substitution);
    if (len_left < (MAX_TEX_LEN - get_operation_tex_len (node->element.value.val_operation_code, cur_len_coefficient)) / 3) return SUCCESS;


    tree_add_substitution (tree_substitution, node->left_son); printf ("plus sub\n");


    return SUCCESS;
}


Return_code  node_substitute_right  (Node* node, Tree_substitution* tree_substitution, double cur_len_coefficient) {

    if (!node || !tree_substitution) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if ( (node->atom_type != DAT_OPERATION) || !node->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    double len_right = 0;
    node_get_tex_len (node->right_son, &len_right, cur_len_coefficient, tree_substitution);


    if (len_right < (MAX_TEX_LEN - get_operation_tex_len (node->element.value.val_operation_code, cur_len_coefficient)) / 3) return SUCCESS;


    node_substitute (node->right_son, tree_substitution, cur_len_coefficient);


    len_right = 0;
    node_get_tex_len (node->left_son, &len_right, cur_len_coefficient, tree_substitution);
    if (len_right < (MAX_TEX_LEN - get_operation_tex_len (node->element.value.val_operation_code, cur_len_coefficient)) / 3) return SUCCESS;


    tree_add_substitution (tree_substitution, node->right_son);


    return SUCCESS;
}


Return_code  tree_add_substitution  (Tree_substitution* tree_substitution, Node* node) {

    if (!tree_substitution || !node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    tree_substitution->substitution_list [tree_substitution->num_substitutions] = { node, tree_substitution->num_substitutions };


    tree_substitution->num_substitutions += 1;


    return SUCCESS;
}


double  get_tex_substitution_len  (Tree_substitution* substitution_list, Node* node) {

    if (!substitution_list || !node) { LOG_ERROR (BAD_ARGS); return NAN; }


    for (size_t i = 0; i < substitution_list->num_substitutions; i++) {

        if ( nodes_the_same (substitution_list->substitution_list [i].node, node) ) {

            double ans = 0;
            ans += get_substitution_letter_len (substitution_list->substitution_list [i].substitution_number);
            return ans;
        }
    }


    return NAN; //not in the list
}


double  get_substitution_letter_len  (size_t substitution_number) {

    double ans   = 0;
    size_t index = (size_t) floor ( (double) substitution_number / ALPHABET_LEN );


    ans += 1; //for letter

    ans += (double) size_t_get_tex_len (index); //for index


    return ans;
}


Node_substitution*  get_tex_substitution  (Tree_substitution* substitutions, Node* node) {

    if (!substitutions || !node) { LOG_ERROR (BAD_ARGS); return nullptr; }


    for (size_t i = 0; i < substitutions->num_substitutions; i++) {

        if ( nodes_the_same (substitutions->substitution_list [i].node, node) ) {

            return &substitutions->substitution_list [i];
        }
    }


    return nullptr;
}


Return_code  tex_write_substitution  (FILE* file, Node_substitution* substitution) {

    if (!file || !substitution) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    tex_write_substitution_letter (file, substitution);
    tex_write_substitution_index  (file, substitution);


    return SUCCESS;
}


Return_code  tex_write_substitution_letter  (FILE* file, Node_substitution* substitution) {

    if (!file || !substitution) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    const char* letter = nullptr;

    switch (substitution->substitution_number % ALPHABET_LEN) {

        case 0:  letter = "\\alpha";      break;
        case 1:  letter = "\\beta";       break;
        case 2:  letter = "\\gamma";      break;
        case 3:  letter = "\\delta";      break;
        case 4:  letter = "\\varepsilon"; break;
        case 5:  letter = "\\zeta";       break;
        case 6:  letter = "\\eta";        break;
        case 7:  letter = "\\theta";      break;
        case 8:  letter = "\\iota";       break;
        case 9:  letter = "\\kappa";      break;
        case 10: letter = "\\lambda";     break;
        case 11: letter = "\\mu";         break;
        case 12: letter = "\\nu";         break;
        case 13: letter = "\\xi";         break;
        case 14: letter = "o";    break;
        case 15: letter = "\\pi";         break;
        case 16: letter = "\\rho";        break;
        case 17: letter = "\\sigma";      break;
        case 18: letter = "\\tau";        break;
        case 19: letter = "\\upsilon";    break;
        case 20: letter = "\\phi";        break;
        case 21: letter = "\\chi";        break;
        case 22: letter = "\\psi";        break;
        case 23: letter = "\\omega";      break;

        default: LOG_MESSAGE ("something's wrong! i can feel it..."); return BAD_ARGS;
    }


    fprintf (file, "%s_", letter);


    return SUCCESS;
}


Return_code  tex_write_substitution_index  (FILE* file, Node_substitution* substitution) {

    if (!file || !substitution) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    size_t index = substitution->substitution_number / ALPHABET_LEN;


    fprintf (file, "{%zd}", index);


    return SUCCESS;
}


Return_code  tex_write_substitutions_decoding (FILE* file, Tree_substitution* substitutions, int precision) {

    if (!file || !substitutions) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!substitutions->num_substitutions) return SUCCESS; //нечего объяснять!


    fprintf (file, "$$\n\n");
    fprintf (file, "\\begin {flushright}");
    fprintf (file, "где\n");


    for (size_t i = 0; i < substitutions->num_substitutions; i++) {

        tex_write_substitution_decoding (file, substitutions, i, precision);
    }


    fprintf (file, "\n");
    fprintf (file, "\\end {flushright}\n$$");


    return SUCCESS;
}


Return_code  tex_write_substitution_decoding  (FILE* file, Tree_substitution* substitutions, size_t index, int precision) {

    if (!file || !substitutions || !substitutions->substitution_list [index].node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    fprintf (file, "$");
    tex_write_substitution (file, &substitutions->substitution_list [index]);
    fprintf (file, " = ");


    Node_substitution this_substitution = substitutions->substitution_list [index]; //чтобы функция печатания не увидила тривиальной замены на эту же букву
    substitutions->substitution_list [index] = { nullptr, 0 };

    tex_write_node (file, this_substitution.node, substitutions, precision);

    substitutions->substitution_list [index] = this_substitution;




    if (true) { //в конце не закрываем! не наша задача index != substitutions->num_substitutions - 1

        fprintf (file, "$\\\\\n");
    }



    return SUCCESS;
}


bool  trees_the_same  (Tree* tree1, Tree* tree2) {

    if (!tree1 || !tree2) { LOG_ERROR (BAD_ARGS); return false; }


    return nodes_the_same (tree1->root, tree2->root);
}


bool  nodes_the_same  (Node* node1, Node* node2) {

    if (!node1 || !node2) { return false; }



    if (node1->atom_type != node2->atom_type) return false;
    if (node1 == node2)                       return true;



    switch (node1->atom_type) {

        case DAT_CONST:

            if ( double_equal (node1->element.value.val_double, node2->element.value.val_double) ) return true;
            return false;

        case DAT_VARIABLE:

            if ( !strcmp (node1->element.value.var_str, node2->element.value.var_str) ) return true;
            return false;

        case DAT_OPERATION: return operation_nodes_the_same (node1, node2);


        default: LOG_ERROR (BAD_ARGS); return false;
    }
}


bool  operation_nodes_the_same  (Node* node1, Node* node2) {

    if (!node1 || !node2 || (node1->atom_type != DAT_OPERATION) || (node2->atom_type != DAT_OPERATION) ) { LOG_ERROR (BAD_ARGS); return false; }


    if (node1->element.value.val_operation_code != node2->element.value.val_operation_code) return false;



    switch (node1->element.value.val_operation_code) {

        case DOC_ADD:  return sons_the_same (node1, node2, true);
        case DOC_SUB:  return sons_the_same (node1, node2, false);
        case DOC_MULT: return sons_the_same (node1, node2, true);
        case DOC_DIV:  return sons_the_same (node1, node2, false);
        case DOC_POW:  return sons_the_same (node1, node2, false);
        case DOC_LN:   return sons_the_same (node1, node2, false);
        case DOC_SIN:  return sons_the_same (node1, node2, false);
        case DOC_COS:  return sons_the_same (node1, node2, false);
        case DOC_TG:   return sons_the_same (node1, node2, false);
        case DOC_CTG:  return sons_the_same (node1, node2, false);

        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return false;
        default:          LOG_ERROR (BAD_ARGS); return false;
    }
}


bool  sons_the_same  (Node* node1, Node* node2, bool is_symmetric_operation) {

    if (!node1           || !node2)            { LOG_ERROR (BAD_ARGS); return false; }
    if (!node1->left_son || !node1->right_son) { LOG_ERROR (BAD_ARGS); return false; }
    if (!node2->left_son || !node2->right_son) { LOG_ERROR (BAD_ARGS); return false; }



    bool ans = false;


    ans |= nodes_the_same (node1->left_son, node2-> left_son) & nodes_the_same (node1->right_son, node2->right_son);


    if (is_symmetric_operation) {

        ans |= nodes_the_same (node1->left_son, node2->right_son) & nodes_the_same (node1->right_son, node2-> left_son);
    }


    return ans;
}


Return_code  dfr_generate_gnu_plot_describtion  (Dfr* dfr, const char* variable, int precision, const char* file_name) {

    if (!dfr || !file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    if (!dfr->user_function_tree) { LOG_MESSAGE ("fill user function first!"); return BAD_ARGS; }
    if (!dfr->tangent)            { LOG_MESSAGE ("fill tangent  first!"); return BAD_ARGS; }



    FILE* file = fopen (file_name, "w"); if (!file) { LOG_ERROR (FILE_ERR); return FILE_ERR; }


    fprintf (file, "set terminal png size 1280, 940 crop\n");
    fprintf (file, "set output \"%s\"\n", dfr_gnu_plot_file_name);
    fprintf (file, "set xlabel \"x\"\n");
    fprintf (file, "set ylabel \"y\"\n");

    fprintf (file, "set title  \"%s(%s) = ", USER_FUNCTION_NAME, variable);
    fclose (file);
    write_function      (dfr->user_function_tree, precision, file_name, "a");
    write_text_function (file_name, "%s", "\"\n");

    write_text_function (file_name, "%s", "plot ");


    write_function      (dfr->user_function_tree, precision, file_name, "a");
    write_text_function (file_name, "%s", ", ");
    write_function      (dfr->tangent, precision, file_name, "a");


    return SUCCESS;
}


Return_code  dfr_calculate_tangent  (Dfr* dfr, const char* variable, double point) {

    if (!dfr || !variable || !isfinite (point) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }

    if (!dfr->derivative_trees_array [0]) { LOG_MESSAGE ("fill up derivative_trees_array [0] first!"); return BAD_ARGS; }


    dfr_ensure_derivative (dfr, variable, 1);



    Tree* shift = (Tree*) calloc (TREE_SIZE, 1); TREE_CTOR (shift);
    Tree* slope = (Tree*) calloc (TREE_SIZE, 1); TREE_CTOR (slope);


    try (tree_evaluate (dfr->derivative_trees_array [0], shift, variable, point));
    try (tree_evaluate (dfr->derivative_trees_array [1], slope, variable, point));



    dfr->tangent = (Tree*) calloc (TREE_SIZE, 1);
    dfr->tangent->root                                  = create_node (DAT_OPERATION, DOC_ADD);
    dfr->tangent->root->left_son                        = shift->root; tree_kill_tree (shift);
    dfr->tangent->root->right_son                       = create_node (DAT_OPERATION, DOC_MULT);
    dfr->tangent->root->right_son->left_son             = slope->root; tree_kill_tree (slope);
    dfr->tangent->root->right_son->right_son            = create_node (DAT_OPERATION, DOC_SUB);
    dfr->tangent->root->right_son->right_son->left_son  = create_node (DAT_VARIABLE, variable);
    dfr->tangent->root->right_son->right_son->right_son = create_node (DAT_CONST, point);


    return SUCCESS;
}


Return_code  dfr_generate_graph  (Dfr* dfr, const char* variable, double point, int precision) {

    if (!dfr || !variable || !isfinite (point)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (dfr_calculate_tangent (dfr, variable, point));
    try (dfr_generate_gnu_plot_describtion (dfr, variable, precision));


    char system_call [MAX_COMMAND_LEN] = "gnuplot ";
    strcat (system_call, dfr_gnu_plot_describtion_file_name);

    system (system_call);


    return SUCCESS;
}


Return_code  tex_write_graph  (FILE* file, Dfr* dfr, const char* variable, double point, int precision) {

    if (!file || !dfr || !variable || !isfinite (point) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    try (dfr_generate_graph (dfr, variable, point, precision));


    fprintf (file, "\\section {график функции}\n");
    fprintf (file, "\\includegraphics [scale=0.6] {%s}", dfr_gnu_plot_file_name);


    return SUCCESS;
}


Return_code  tex_generate_pdf  (const char* file_name) {

    if (!file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    char command [MAX_COMMAND_LEN] = "pdflatex ";
    strcat (command, dfr_default_tex_file_name);
    strcat (command, " -quiet");
    strcat (command, " -output-directory work");


    system (command);


    return SUCCESS;
}


Return_code  _dfr_sorry_message  (const char* file, const char* func, int line) {

    printf ("\n-------------------------------------------------\n");
    printf ("something went wrong... please, check your input!\n");
    printf ("(file %s, function %s, line %d)\n", file, func, line);
    printf ("-------------------------------------------------\n\n");


    return SUCCESS;
}


Return_code  dfr_buffer_dump  (Dfr_buffer* buffer) {

    if (!buffer) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    for (size_t i = 0; i < buffer->len; i++) {

        printf ("%zd - ", i);


        switch (buffer->buffer [i].atom_type) {

            case DAT_CONST:     printf ("%lf", buffer->buffer [i].val_double);                break;
            case DAT_VARIABLE:  printf ("%s", buffer->buffer [i].val_str);                   break;
            case DAT_OPERATION: printf ("%d",  buffer->buffer [i].val_buffer_operation_code); break;

            default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        }


        printf ("\n");
    }


    return SUCCESS;
}


//--------------------------------------------------
#define LEFT  node->left_son
#define OP    element.value.val_operation_code
#define CONST element.value.val_double
//--------------------------------------------------


bool  is_unary  (Node* node) {

    if (!node) { LOG_ERROR (BAD_ARGS); return false; }


    if (node->atom_type != DAT_OPERATION) return false;

    switch (node->OP) {

        case DOC_ADD:  return false;
        case DOC_MULT: return false;
        case DOC_DIV:  return false;
        case DOC_POW:  return false;

        case DOC_SUB: break;
        case DOC_LN:  break;
        case DOC_SIN: break;
        case DOC_COS: break;
        case DOC_TG:  break;
        case DOC_CTG: break;

        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return false;
        default: break;
    }


    if (!LEFT) { LOG_ERROR (BAD_ARGS); return false; }


    if (node->OP == DOC_SUB && (LEFT->atom_type != DAT_CONST || !double_equal (LEFT->CONST, 0))) return false;


    return true;
}

bool  is_unary_minus  (Node* node) {

    if (!node) { LOG_ERROR (BAD_ARGS); return false; }


    if (node->atom_type != DAT_OPERATION) return false;


    if ((node->OP == DOC_SUB) && (LEFT->atom_type == DAT_CONST && double_equal (LEFT->CONST, 0))) return true;


    return false;
}

