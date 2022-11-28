

#include "../lib/tree_dfr.hpp"

#include "../lib/double_compare.hpp"

#include "../headers/dfr.hpp"


//--------------------------------------------------


Return_code  _dfr_ctor  (Dfr* dfr, const char* name, const char* file, const char* func, int line) {

    assert ( (file) && (name) && (func) && (line > 0) );
    if (dfr == nullptr) { LOG_ERROR (BAD_ARGS); DFR_ERROR_DUMP (dfr); return BAD_ARGS; }


    dfr->user_function_tree = (Tree*) calloc (TREE_SIZE, 1);
    TREE_CTOR (dfr->user_function_tree);

    dfr->derivative_tree =    (Tree*) calloc (TREE_SIZE, 1);
    TREE_CTOR (dfr->derivative_tree);

    dfr->buffer = (Dfr_buffer*) calloc (DFR_BUFFER_SIZE, 1);
    dfr_buffer_ctor (dfr->buffer);


    dfr->debug_info.name       = name;
    dfr->debug_info.birth_file = file;
    dfr->debug_info.birth_func = func;
    dfr->debug_info.birth_line = line;
    dfr->debug_info.adress     = dfr;


    DFR_AFTER_OPERATION_DUMPING (dfr);


    return SUCCESS;
}


Return_code  dfr_dtor  (Dfr* dfr) {

    if (!dfr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (dfr->user_function_tree) free (dfr->user_function_tree);
    if (dfr->derivative_tree)    free (dfr->derivative_tree);

    dfr_buffer_dtor (dfr->buffer);


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
        /*case DOC_POW:  return 1;
        case DOC_SIN:  return 2;
        case DOC_COS:  return 2;*/

        default: LOG_ERROR (BAD_ARGS); return 0;
    }
}


Return_code  read_number  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || ((*buffer_node_ptr)->atom_type != DAT_CONST)
                         || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


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


Return_code  read_primary  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if ( (*buffer_node_ptr)->atom_type == DAT_OPERATION &&  (*buffer_node_ptr)->val_buffer_operation_code == DBOC_OPEN_BRACKET) {

        *buffer_node_ptr += 1;

        read_sum (buffer_node_ptr, max_buffer_node, node_ptr);

        *buffer_node_ptr += 1;
    }

    else if ( (*buffer_node_ptr)->atom_type == DAT_VARIABLE ) { read_variable (buffer_node_ptr, max_buffer_node, node_ptr); }
    else                                                      { read_number   (buffer_node_ptr, max_buffer_node, node_ptr); }


    return SUCCESS;
}


Return_code  read_product  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    read_primary (buffer_node_ptr, max_buffer_node, node_ptr);


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

        read_primary (buffer_node_ptr, max_buffer_node, &(*node_ptr)->right_son);
    }


    return SUCCESS;
}


Return_code  read_sum  (Buffer_node** buffer_node_ptr, Buffer_node* max_buffer_node, Node** node_ptr) {

    if (!buffer_node_ptr || (*buffer_node_ptr >= max_buffer_node) || !node_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    read_product (buffer_node_ptr, max_buffer_node, node_ptr);


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

        read_product (buffer_node_ptr, max_buffer_node, &(*node_ptr)->right_son);
    }


    return SUCCESS;
}


Return_code  read_general  (Dfr_buffer* dfr_buffer, Tree* tree) {

    if (!dfr_buffer || !tree) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Buffer_node* current_buffer_node = dfr_buffer->buffer;


    read_sum (&current_buffer_node, dfr_buffer->buffer + dfr_buffer->len, &tree->root);


    assert (current_buffer_node == dfr_buffer->buffer + dfr_buffer->len);


    return SUCCESS;
}


Return_code  dfr_read_user_function  (Dfr* dfr, const char* file_name) {

    if (!dfr || !dfr->user_function_tree || !file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    FILE* file = fopen (file_name, "r");
    if (!file) { LOG_ERROR (FILE_ERR); return FILE_ERR; }


    size_t file_len = get_file_len (file);
    char buffer [file_len + 1] = "";
    fread (buffer, 1, file_len, file);
    fclose (file);


    _dfr_buffer_read  ( dfr->buffer, buffer);
    try (read_general (dfr->buffer, dfr->user_function_tree));


    return SUCCESS;
}


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


Return_code  dfr_write_user_function  (Dfr* dfr, const char* file_name) {

    if (!dfr || !file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    write_function (dfr->user_function_tree, file_name, "your function: " );


    return SUCCESS;
}


Return_code  dfr_write_derivative  (Dfr* dfr, const char* file_name) {

    if (!dfr || !file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    write_function (dfr->derivative_tree, file_name, "derivative:    ");


    return SUCCESS;
}


Return_code  write_function  (Tree* tree, const char* file_name, const char* additional_text) {

    if (!tree || !file_name) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    static bool first_time_writing = true;

    const char* file_mode = nullptr;

    if (first_time_writing) { file_mode = "w"; }
    else                    { file_mode = "a"; }

    FILE* file = fopen (file_name, file_mode);
    if (file == nullptr) { LOG_ERROR (FILE_ERR); return FILE_ERR; }



    fprintf (file, "%s", additional_text);


    Tree_iterator tree_iterator = {};
    tree_iterator_ctor (&tree_iterator, tree, "my_mode");


    write_function_dive_left (tree, &tree_iterator, file);


    do {

        switch (tree_iterator.current->atom_type) {

            case DAT_OPERATION: fprintf (file, " %s ", _operation_code_to_str (tree_iterator.current->element.value.val_operation_code)); break;
            case DAT_CONST:     fprintf (file, "%.1lf",                      tree_iterator.current->element.value.val_double);          break;
            case DAT_VARIABLE:  fprintf (file, "%s",                         tree_iterator.current->element.value.var_str);             break;

            default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        }

    } while (!write_function_inc (tree, &tree_iterator, file));


    fprintf (file, "\n");


    first_time_writing = false;


    fclose (file);


    return SUCCESS;
}


Return_code  write_function_dive_left  (Tree* tree, Tree_iterator* tree_iterator, FILE* file) {

    if (!tree || !tree_iterator || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    stack_push (tree_iterator->node_stack, tree_iterator->current);
    tree_iterator->depth = 0;


    while (tree_iterator->current->left_son) {

        write_function_check_open_bracket (tree_iterator, "L", file);


        tree_iterator->current = tree_iterator->current->left_son;
        stack_push (tree_iterator->node_stack, tree_iterator->current);
        tree_iterator->depth += 1;
    }


    return SUCCESS;
}


Return_code  write_function_inc  (Tree* tree, Tree_iterator* tree_iterator, FILE* file) {

    if (!tree || !tree_iterator || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (tree_iterator->current->right_son) {

        write_function_check_open_bracket (tree_iterator, "R", file);


        tree_iterator->current = tree_iterator->current->right_son;
        stack_push (tree_iterator->node_stack, tree_iterator->current);
        tree_iterator->depth += 1;

        while (tree_iterator->current->left_son) {

            write_function_check_open_bracket (tree_iterator, "L", file);


            tree_iterator->current = tree_iterator->current->left_son;
            stack_push (tree_iterator->node_stack, tree_iterator->current);
            tree_iterator->depth += 1;
        }

        tree_iterator->index += 1;
        return SUCCESS;
    }


    if (tree_iterator->depth == 0) { return BAD_ARGS; } //дошли до корня, справа нет узлов


    write_function_check_closing_bracket (tree_iterator, file);


    Node* old = tree_iterator->current;
    stack_pop (tree_iterator->node_stack);
    tree_iterator->current = (Node*) stack_pop (tree_iterator->node_stack).value;
    tree_iterator->depth -= 1;
    stack_push (tree_iterator->node_stack, tree_iterator->current);



    while (tree_iterator->current->right_son == old) {//пришел справа

        if (tree_iterator->depth == 0) { return BAD_ARGS; } // already was at last element


        write_function_check_closing_bracket (tree_iterator, file);


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


Return_code  write_function_check_open_bracket  (Tree_iterator* tree_iterator, const char* next_node_str, FILE* file) {

    if (!tree_iterator || !file || !next_node_str || (stricmp (next_node_str, "L") && stricmp (next_node_str, "R")) ) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Node* son = nullptr;

    if (!stricmp (next_node_str, "L")) { son = tree_iterator->current->left_son;  }
    else                               { son = tree_iterator->current->right_son; }


    if (tree_iterator->current->atom_type == DAT_OPERATION && son->atom_type == DAT_OPERATION &&
           get_operation_priority (tree_iterator->current->element.value.val_operation_code) >
           get_operation_priority (son->                   element.value.val_operation_code)) {

        fprintf (file, "(");
    }


    return SUCCESS;
}


Return_code  write_function_check_closing_bracket  (Tree_iterator* tree_iterator, FILE* file) {

    if (!tree_iterator || !file) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Node* son    = (Node*) stack_pop  (tree_iterator->node_stack).value;
    Node* parent = (Node*) stack_pop  (tree_iterator->node_stack).value;
                           stack_push (tree_iterator->node_stack, parent);
                           stack_push (tree_iterator->node_stack, son);


    if (son->atom_type == DAT_OPERATION && parent->atom_type == DAT_OPERATION &&
           get_operation_priority (son   ->element.value.val_operation_code) <
           get_operation_priority (parent->element.value.val_operation_code)) {

        fprintf (file, ")");
    }


    return SUCCESS;
}


Return_code  dfr_calculate_derivative_tree  (Dfr* dfr, const char* variable) {

    if (!dfr || !variable) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    dfr->derivative_tree->root = node_calculate_derivative_tree (dfr->user_function_tree->root, variable);


    tree_fold (dfr->derivative_tree);


    return SUCCESS;
}


Node*  node_calculate_derivative_tree  (Node* node, const char* variable) {

    switch (node->atom_type) {

        case DAT_OPERATION: return operation_calculate_derivative_tree (node, variable);
        case DAT_CONST:     return create_node (DAT_CONST, (double) 0);
        case DAT_VARIABLE:  return variable_calculate_derivative_tree  (node, variable);

        default: LOG_ERROR (BAD_ARGS); return nullptr;
    }
}


Node*  operation_calculate_derivative_tree  (Node* node, const char* variable) {

    if (!node || !variable || (node->atom_type != DAT_OPERATION)) { LOG_ERROR (BAD_ARGS); return nullptr; }


    Node* root_node = nullptr;

    switch (node->element.value.val_operation_code) {

        case DOC_UNKNOWN:

            LOG_ERROR (BAD_ARGS); root_node = create_node (DAT_OPERATION, DOC_UNKNOWN); break;

        case DOC_ADD:

            root_node = create_node (DAT_OPERATION, DOC_ADD);

            root_node->left_son  = node_calculate_derivative_tree (node->left_son,  variable);
            root_node->right_son = node_calculate_derivative_tree (node->right_son, variable);

            break;

        case DOC_SUB:

            root_node = create_node (DAT_OPERATION, DOC_SUB);

            root_node->left_son  = node_calculate_derivative_tree (node->left_son,  variable);
            root_node->right_son = node_calculate_derivative_tree (node->right_son, variable);

            break;

        case DOC_MULT:

            root_node = create_node (DAT_OPERATION, DOC_ADD);

            root_node->left_son  = create_node (DAT_OPERATION, DOC_MULT);
            root_node->right_son = create_node (DAT_OPERATION, DOC_MULT);

            root_node->left_son->left_son   = node_calculate_derivative_tree (node->left_son,  variable);
            root_node->left_son->right_son  = copy_node (node->right_son);
            root_node->right_son->left_son  = copy_node (node-> left_son);
            root_node->right_son->right_son = node_calculate_derivative_tree (node->right_son, variable);

            break;

        case DOC_DIV:

            root_node = create_node (DAT_OPERATION, DOC_DIV);

            root_node->left_son  = create_node (DAT_OPERATION, DOC_SUB);
            root_node->right_son = create_node (DAT_OPERATION, DOC_MULT);

            root_node->left_son->left_son   = create_node (DAT_OPERATION, DOC_MULT);
            root_node->left_son->left_son->left_son  = node_calculate_derivative_tree (node->left_son, variable);
            root_node->left_son->left_son->right_son = copy_node (node->right_son);
            root_node->left_son->right_son = create_node (DAT_OPERATION, DOC_MULT);
            root_node->left_son->right_son->left_son  = copy_node (node->left_son);
            root_node->left_son->right_son->right_son = node_calculate_derivative_tree (node->right_son, variable);

            root_node->right_son->left_son  = copy_node (node->right_son);
            root_node->right_son->right_son = copy_node (node->right_son);

            break;


        default:

            LOG_ERROR (BAD_ARGS); return nullptr;
    }

    return root_node;
}


Node*  variable_calculate_derivative_tree  (Node* node, const char* variable) {

    if (!node || !variable || (node->atom_type != DAT_VARIABLE)) { LOG_ERROR (BAD_ARGS); return nullptr; }


    if ( !strcmp (node->element.value.var_str, variable) ) { return create_node (DAT_CONST, (double) 1); }


    return create_node (DAT_CONST, (double) 0);
}


bool  tree_fold_constants  (Tree* tree) {

    if (!tree) { LOG_ERROR (BAD_ARGS); return false; }


    return node_fold_constants (tree->root);
}


bool  node_fold_constants  (Node* node) {

    if (!node) { LOG_ERROR (BAD_ARGS); return false; }


    bool folded_anything = 0;


    if (node-> left_son) { folded_anything |= node_fold_constants (node-> left_son); }
    if (node->right_son) { folded_anything |= node_fold_constants (node->right_son); }


    switch (node->atom_type) {

        case DAT_OPERATION: folded_anything |= operation_fold_constants (node); break;
        case DAT_CONST:     break;
        case DAT_VARIABLE:  break;

        default: LOG_ERROR (BAD_ARGS); return false;
    }


    return folded_anything;
}


bool  operation_fold_constants  (Node* node) {

    if (!node || (node->atom_type != DAT_OPERATION) ) { LOG_ERROR (BAD_ARGS); return false; }


    if (node->left_son->atom_type != DAT_CONST || node->right_son->atom_type != DAT_CONST) { return false; }


    switch (node->element.value.val_operation_code) {

        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return false;

        case DOC_ADD:

            node->element.value.val_double = node->left_son->element.value.val_double + node->right_son->element.value.val_double;
            break;

        case DOC_SUB:

            node->element.value.val_double = node->left_son->element.value.val_double - node->right_son->element.value.val_double;
            break;

        case DOC_MULT:

            node->element.value.val_double = node->left_son->element.value.val_double * node->right_son->element.value.val_double;
            break;

        case DOC_DIV:

            node->element.value.val_double = node->left_son->element.value.val_double / node->right_son->element.value.val_double;
            break;


        default: LOG_ERROR (BAD_ARGS); return false;
    }


    node->atom_type = DAT_CONST;


    node_dtor (node-> left_son);
    node_dtor (node->right_son);
    node->right_son = nullptr;
    node-> left_son = nullptr;


    return true;
}


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


bool  tree_fold_neutral  (Tree* tree) {

    if (!tree) { LOG_ERROR (BAD_ARGS); return false; }


    return node_fold_neutral (tree->root);
}


bool  node_fold_neutral  (Node* node) {

    if (!node) { LOG_ERROR (BAD_ARGS); return false; }


    bool folded_anything = 0;


    if (node-> left_son) { folded_anything |= node_fold_neutral (node-> left_son); }
    if (node->right_son) { folded_anything |= node_fold_neutral (node->right_son); }


    switch (node->atom_type) {

        case DAT_OPERATION: folded_anything |= operation_fold_neutral (node); break;
        case DAT_CONST:     break;
        case DAT_VARIABLE:  break;

        default: LOG_ERROR (BAD_ARGS); return false;
    }


    return folded_anything;
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
    return true;

#define PROMOTE_RIGHT\
    _node_dtor (LEFT);\
    node_realloc (RIGHT, node);\
    return true;

#define NODE_TO_0\
    _node_dtor (LEFT);\
    _node_dtor (RIGHT);\
\
    node->atom_type = DAT_CONST;\
    node->CONST     = 0;\
    node-> left_son = nullptr;\
    node->right_son = nullptr;\
    return true;


//--------------------------------------------------


bool  operation_fold_neutral  (Node* node) {

    if (!node || (node->atom_type != DAT_OPERATION) ) { LOG_ERROR (BAD_ARGS); return false; }


    switch (node->OPERATION_CODE) {

        case DOC_UNKNOWN: LOG_ERROR (BAD_ARGS); return false;

        case DOC_ADD:

            if (LEFT-> atom_type == DAT_CONST && double_equal (LEFT-> CONST, 0) ) { PROMOTE_RIGHT }
            if (RIGHT->atom_type == DAT_CONST && double_equal (RIGHT->CONST, 0) ) { PROMOTE_LEFT }

            break;

        case DOC_SUB:

            if (RIGHT->atom_type == DAT_CONST && double_equal (RIGHT->CONST, 0) ) { PROMOTE_LEFT }

            break;

        case DOC_MULT:

            if (LEFT-> atom_type == DAT_CONST && double_equal (LEFT-> CONST, 1) ) { PROMOTE_RIGHT }
            if (RIGHT->atom_type == DAT_CONST && double_equal (RIGHT->CONST, 1) ) { PROMOTE_LEFT }

            if (LEFT-> atom_type == DAT_CONST && double_equal (LEFT-> CONST, 0) ) { NODE_TO_0 }
            if (RIGHT->atom_type == DAT_CONST && double_equal (RIGHT->CONST, 0) ) { NODE_TO_0 }

            break;

        case DOC_DIV:

            if (RIGHT->atom_type == DAT_CONST && double_equal (RIGHT->CONST, 1) ) { PROMOTE_LEFT }

            if (LEFT-> atom_type == DAT_CONST && double_equal (LEFT-> CONST, 0) ) { NODE_TO_0 }

            break;


        default: LOG_ERROR (BAD_ARGS); return false;
    }


    return false;
}


Return_code  tree_fold  (Tree* tree) {

    if (!tree) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    bool folded_anything = true;

    while (folded_anything) {

        folded_anything = false;

        folded_anything |= tree_fold_constants (tree);
        folded_anything |= tree_fold_neutral   (tree);
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
//printf ("str - %s\n", str);
        read_buffer_operation (&str, &dfr_buffer->buffer [cur_node]);
        skipspaces (&str);
        cur_node += 1;
    }

    dfr_buffer->len = cur_node;


    return SUCCESS;
}


Return_code  read_buffer_operation  (char** str_ptr, Buffer_node* buffer_node) {

    if (!str_ptr || !*str_ptr) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (**str_ptr == '(') { *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_OPEN_BRACKET };    return SUCCESS; }
    if (**str_ptr == ')') { *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_CLOSING_BRACKET }; return SUCCESS; }
    if (**str_ptr == '+') { *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_ADD };             return SUCCESS; }
    if (**str_ptr == '-') { *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_SUB };             return SUCCESS; }
    if (**str_ptr == '*') { *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_MULT };            return SUCCESS; }
    if (**str_ptr == '/') { *str_ptr += 1; *buffer_node = { DAT_OPERATION, .val_buffer_operation_code = DBOC_DIV };             return SUCCESS; }


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
    
        free (&dfr_buffer->buffer [i]);
    }


    free (dfr_buffer->buffer);


    return SUCCESS;
}

