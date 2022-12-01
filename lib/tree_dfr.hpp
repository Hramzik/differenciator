#ifndef TREE_HPP_INCLUDED
#define TREE_HPP_INCLUDED


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


#include "onegin.hpp"
#include "stack.hpp"


#include "types/Elements/use_Element_atom.hpp"
#include "types/Elements/Element_atom.hpp"






#include "types/Tree_dfr.hpp"


//--------------------------------------------------


static char GLOBAL_graph_dump_num [MAX_GRAPH_DUMP_NUM] = "1";


Return_code  _tree_ctor  (Tree* tree, const char* name, const char* file, const char* func, int line) {

    assert ( (file) && (name) && (func) && (line > 0) );
    if (tree == nullptr) { LOG_ERROR (BAD_ARGS); TREE_ERROR_DUMP (tree); return BAD_ARGS; }


    tree->root = (Node*) calloc (NODE_SIZE, 1);

    tree->root->atom_type = DAT_OPERATION;
    tree->root->element   = { _poisoned_Element_value, false };
    tree->root->left_son  = nullptr;
    tree->root->right_son = nullptr;

    tree->root->isnew       = true;
    tree->root->birth_line  = 0;
    tree->root->birth_index = 0;


    tree->debug_info.name       = name;
    tree->debug_info.birth_file = file;
    tree->debug_info.birth_func = func;
    tree->debug_info.birth_line = line;
    tree->debug_info.adress     = tree;


    TREE_AFTER_OPERATION_DUMPING (tree);


    return SUCCESS;
}


Node*  create_node  (Atom_type atom_type, ...) {

    va_list value;
    va_start (value, atom_type);


    Node* node = (Node*) calloc (NODE_SIZE, 1); if (!node) { LOG_ERROR (MEMORY_ERR); return nullptr; }

    node->atom_type = atom_type;
    node->left_son  = nullptr;
    node->right_son = nullptr;
    node->element.poisoned = false;


    switch (atom_type) {

        case DAT_OPERATION: node->element.value.val_operation_code = (Operation_code) va_arg(value, int);    break;
        case DAT_CONST:     node->element.value.val_double         =                  va_arg(value, double); break;
        case DAT_VARIABLE:  node->element.value.var_str            =                  va_arg(value, char*);  break;

        default: LOG_ERROR (BAD_ARGS); return nullptr;
    }


    va_end (value);


    return node;
}


Return_code  tree_kill_root  (Tree* tree) {

    if (!tree || !tree->root || tree->root->left_son || tree->root->right_son) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    free (tree->root);


    return SUCCESS;
}


Return_code  tree_kill_tree  (Tree* tree) {

    if (!tree) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    free (tree);


    return SUCCESS;
}



Return_code  tree_dtor  (Tree* tree) {

    ASSERT_TREE_OK (tree);


    _node_dtor (tree->root);


    return SUCCESS;
}


Return_code  _node_dtor  (Node* node) {

    if (!node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (node->left_son)  { try (_node_dtor (node->left_son));  }
    if (node->right_son) { try (_node_dtor (node->right_son)); }


    free (node);


    return SUCCESS;
}


Return_code  node_dtor  (Node* node) {

    if (!node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    free (node);


    return SUCCESS;
}


Return_code  node_realloc  (Node* old_node, Node* new_node) {

    if (!old_node || !new_node) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    *new_node = *old_node;


    node_dtor (old_node);


    return SUCCESS;
}


Return_code  tree_push_left  (Tree* tree, Node* node, Element_value new_element_value, Atom_type atom_type, bool isnew, ...) {

    if (!tree || !node) { LOG_ERROR (BAD_ARGS); FTREE_DUMP (tree); return BAD_ARGS; };

    node->left_son = (Node*) calloc (NODE_SIZE, 1);


    switch (atom_type) {

        case DAT_OPERATION: node->left_son->element   = { { .val_operation_code = new_element_value }, false }; break;
        case DAT_CONST:     node->left_son->element   = { { .val_double         = new_element_value }, false }; break;
        case DAT_VARIABLE:  node->left_son->element   = { { .var_str            = new_element_value }, false }; break;

        default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }

    node->left_son->atom_type = atom_type;
    node->left_son->left_son  = nullptr;
    node->left_son->right_son = nullptr;


    node->isnew = isnew;

    if (isnew) {

        node->left_son->birth_line  = 0;
        node->left_son->birth_index = 0;
    }

    else {

        va_list birth_info;
        va_start (birth_info, isnew);
        node->left_son->birth_line  = va_arg (birth_info, size_t);
        node->left_son->birth_index = va_arg (birth_info, size_t);
        va_end (birth_info);
    }


    return SUCCESS;
}


Return_code  tree_push_right  (Tree* tree, Node* node, Element_value new_element_value, Atom_type atom_type, bool isnew, ...) {

    if (!tree || !node) { LOG_ERROR (BAD_ARGS); FTREE_DUMP (tree); return BAD_ARGS; };

    node->right_son = (Node*) calloc (NODE_SIZE, 1);


    switch (atom_type) {

        case DAT_OPERATION: node->left_son->element   = { { .val_operation_code = new_element_value }, false }; break;
        case DAT_CONST:     node->left_son->element   = { { .val_double         = new_element_value }, false }; break;
        case DAT_VARIABLE:  node->left_son->element   = { { .var_str            = new_element_value }, false }; break;

        default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
    }

    node->right_son->atom_type = atom_type;
    node->right_son->left_son  = nullptr;
    node->right_son->right_son = nullptr;


    node->isnew = isnew;

    if (isnew) {

        node->right_son->birth_line  = 0;
        node->right_son->birth_index = 0;
    }

    else {

        va_list birth_info;
        va_start (birth_info, isnew);
        node->right_son->birth_line  = va_arg (birth_info, size_t);
        node->right_son->birth_index = va_arg (birth_info, size_t);
        va_end (birth_info);
    }


    return SUCCESS;
}


Tree_state  tree_damaged  (Tree* tree) {

    Tree_state tree_state = TR_OK;


    if (!tree) { tree_state |= TR_NULLPTR; return tree_state; }


    if (!tree->root) { tree_state |= TR_NULLPTR_ROOT; }

    tree_state |= _tree_poison_distribution (tree);

    return tree_state;
}


Tree_state _tree_poison_distribution (Tree* tree) {

    return _node_poison_distribution (tree->root);
}


Tree_state _node_poison_distribution (Node* node) {

    if (!node) { return TR_OK; }


    if (node->element.poisoned) {

        return TR_INCORRECT_POISON_DISTRIBUTION;
    }

    if ( node->left_son && _node_poison_distribution (node->left_son) ) {

        return TR_INCORRECT_POISON_DISTRIBUTION;
    }

    if ( node->right_son && _node_poison_distribution (node->right_son) ) {

        return TR_INCORRECT_POISON_DISTRIBUTION;
    }


    return TR_OK;
}


void  _ftree_dump  (Tree* tree, const char* file_name, const char* file, const char* func, int line, const char* file_mode) {

    assert ( (file_name) && (file) && (func) && (line > 0) );


    static bool first_time_dumping = true;

    if (strcmp (file_mode, "w") && strcmp (file_mode, "a")) {

        if (first_time_dumping) { file_mode = "w"; }
        else                    { file_mode = "a"; }
    }


    FILE* dump_file = fopen (file_name, file_mode);
    if (dump_file == nullptr) { LOG_ERROR (FILE_ERR); return; }


    setvbuf (dump_file, NULL, _IONBF, 0);


    fprintf (dump_file, "--------------------\n");
    fprintf (dump_file, "Dumping tree at %s in function %s (line %d)...\n\n", file, func, line);


    if (!tree) { fprintf (dump_file, "Tree pointer is nullptr!\n\n"); return; }


    fprintf (dump_file, "this tree has name ");
    if (tree->debug_info.name != nullptr) { fprintf (dump_file, "%s ", tree->debug_info.name); }
    else                                  { fprintf (dump_file, "UNKNOWN NAME "); }
    fprintf (dump_file, "[%p]\n", tree->debug_info.adress);

    fprintf (dump_file, "it was created in file ");
    if (tree->debug_info.birth_file != nullptr) { fprintf (dump_file, "%s\n", tree->debug_info.birth_file); }
    else                                        { fprintf (dump_file, "UNKNOWN NAME\n"); }

    fprintf (dump_file, "in function ");
    if (tree->debug_info.birth_func != nullptr) { fprintf (dump_file, "%s ", tree->debug_info.birth_func); }
    else                                        { fprintf (dump_file, "UNKNOWN NAME "); }

    fprintf (dump_file, "(line %d)\n\n", tree->debug_info.birth_line);


    fprintf (dump_file, "tree is ");
    Tree_state tree_state = tree_damaged (tree);
    if (tree_state) { fprintf (dump_file, "damaged (damage code %u)\n", tree_state); }
    else            { fprintf (dump_file, "ok\n"); }


    if (tree->root) { fprintf (dump_file, "root [%p].\n\n", tree->root); }
    else            { fprintf (dump_file, "root [nullptr].\n\n"); }



    fprintf (dump_file, "nodes:\n\n\n");

    fprintf (dump_file, "pre-order:\n\n");
    _fdump_nodes (dump_file, tree, "pre");

    fprintf (dump_file, "\n\nin-order:\n\n");
    _fdump_nodes (dump_file, tree, "in");


    fprintf (dump_file, "\n");


    fclose (dump_file);


    first_time_dumping = false;
}


Return_code  tree_iterator_inc  (Tree_iterator* tree_iterator) {

    if (!tree_iterator || !tree_iterator->mode) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (!strcmp (tree_iterator->mode, "in"))  { return _tree_iterator_inc_in_order  (tree_iterator); }
    if (!strcmp (tree_iterator->mode, "pre")) { return _tree_iterator_inc_pre_order (tree_iterator); }


    LOG_ERROR (BAD_ARGS);
    return BAD_ARGS;
}


Return_code  _tree_iterator_inc_in_order  (Tree_iterator* tree_iterator) {

    if (!tree_iterator) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    if (tree_iterator->current->right_son) {

        tree_iterator->current = tree_iterator->current->right_son;
        stack_push (tree_iterator->node_stack, tree_iterator->current);
        tree_iterator->depth += 1;

        while (tree_iterator->current->left_son) {

            tree_iterator->current = tree_iterator->current->left_son;
            stack_push (tree_iterator->node_stack, tree_iterator->current);
            tree_iterator->depth += 1;
        }

        tree_iterator->index += 1;
        return SUCCESS;
    }


    if (tree_iterator->depth == 0) { return BAD_ARGS; } //дошли до корня, справа нет узлов


    Node* old = tree_iterator->current;
    stack_pop (tree_iterator->node_stack);
    tree_iterator->current = (Node*) stack_pop (tree_iterator->node_stack).value;
    tree_iterator->depth -= 1;
    stack_push (tree_iterator->node_stack, tree_iterator->current);



    while (tree_iterator->current->right_son == old) {//пришел справа

        if (tree_iterator->depth == 0) { return BAD_ARGS; } // already was at last element


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


Return_code  _tree_iterator_inc_pre_order  (Tree_iterator* tree_iterator) {

    if (!tree_iterator) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }



    if (tree_iterator->current->left_son) {

        tree_iterator->current = tree_iterator->current->left_son;
        stack_push (tree_iterator->node_stack, tree_iterator->current);
        tree_iterator->depth += 1;
        tree_iterator->index += 1;
        return SUCCESS;
    }



    Node* old = tree_iterator->current;
    while (true) {

        old = tree_iterator->current;
        stack_pop (tree_iterator->node_stack);
        tree_iterator->current = (Node*) stack_pop (tree_iterator->node_stack).value;
        stack_push (tree_iterator->node_stack, tree_iterator->current);


        if (!tree_iterator->current) { return BAD_ARGS; } //pop returned nullptr <=> root was already popped <=> chain was started by last node


        tree_iterator->depth -= 1;


        if (tree_iterator->current->left_son == old) { //пришли слева

            stack_push (tree_iterator->node_stack, tree_iterator->current->right_son);
            tree_iterator->current = tree_iterator->current->right_son;

            tree_iterator->depth += 1;
            break;
        }
    }


    tree_iterator->index += 1;
    return SUCCESS;
}


Return_code  tree_iterator_ctor  (Tree_iterator* tree_iterator, Tree* tree, const char* mode) {

    if (!tree_iterator || !mode || tree_damaged (tree)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    tree_iterator->node_stack = (Stack*) calloc (STACK_SIZE, 1);
    tree_iterator->current    = tree->root;
    STACK_CTOR (tree_iterator->node_stack);

    if (!strcmp (mode, "in")) {

        _tree_iterator_ctor_fill_stack_in_order (tree_iterator, tree);
    }

    if (!strcmp (mode, "pre")) {

        _tree_iterator_ctor_fill_stack_pre_order (tree_iterator, tree);
    }


    tree_iterator->index = 0;


    tree_iterator->mode = mode;


    return SUCCESS;
}


Return_code  _tree_iterator_ctor_fill_stack_in_order  (Tree_iterator* tree_iterator, Tree* tree) {

    if (!tree_iterator || tree_damaged (tree)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    tree_iterator->current = tree->root;


    stack_push (tree_iterator->node_stack, tree_iterator->current);
    tree_iterator->depth = 0;


    while (tree_iterator->current->left_son) {

        tree_iterator->current = tree_iterator->current->left_son;
        stack_push (tree_iterator->node_stack, tree_iterator->current);
        tree_iterator->depth += 1;
    }


    return SUCCESS;
}


Return_code  _tree_iterator_ctor_fill_stack_pre_order  (Tree_iterator* tree_iterator, Tree* tree) {

    if (!tree_iterator || tree_damaged (tree)) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    tree_iterator->current = tree->root;


    stack_push (tree_iterator->node_stack, tree_iterator->current);
    tree_iterator->depth = 0;


    return SUCCESS;
}


Return_code  tree_iterator_dtor (Tree_iterator* tree_iterator) {

    if (!tree_iterator) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    stack_dtor (tree_iterator->node_stack);
    free       (tree_iterator->node_stack);


    return SUCCESS;
}


bool  _isleaf  (Node* node) {

    assert (node);


    if (!node->left_son && !node->right_son) { return true; }


    return false;
}


void  _fprint_tabs  (FILE* file, size_t num) {

    for (size_t i = 0; i < num; i++) {
    
        fprintf (file, "    ");
    }
}


Return_code  _fdump_nodes  (FILE* dump_file, Tree* tree, const char* mode) {

    if (!dump_file || !tree || !mode) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    Tree_iterator tree_iterator = {};
    tree_iterator_ctor (&tree_iterator, tree, mode);

    do {

        _fprint_tabs (dump_file, tree_iterator.depth);

        fprintf (dump_file, "[%zd] = ", tree_iterator.index);

        switch (tree_iterator.current->atom_type) {

            case DAT_OPERATION: fprintf (dump_file,  "%d (aka \"%s\") (type Operation_code)",
                                                                         tree_iterator.current->element.value.val_operation_code,
                                                 _operation_code_to_str (tree_iterator.current->element.value.val_operation_code)
                                        ); break;

            case DAT_CONST:     fprintf (dump_file, "%lf (type double)", tree_iterator.current->element.value.val_double);         break;
            case DAT_VARIABLE:  fprintf (dump_file,  "%s (type string)", tree_iterator.current->element.value.var_str);            break;

            default: LOG_ERROR (BAD_ARGS); return BAD_ARGS;
        }

        fprintf (dump_file, " (depth %zd)\n", tree_iterator.depth);
    }
    while (!tree_iterator_inc (&tree_iterator));

    tree_iterator_dtor (&tree_iterator);


    return SUCCESS;
}


void  _ftree_graphdump  (Tree* tree, const char* file_name, const char* file, const char* func, int line, const char* additional_text) {

    assert ( (file_name) && (file) && (func) && (line > 0) );


    char file_path [MAX_COMMAND_LEN] = "";
    strcat (file_path, file_name);
    strcat (file_path, ".html");


    const char* file_mode = nullptr;
    if ( !strcmp (GLOBAL_graph_dump_num, "1")) { file_mode = "w"; }
    else                                       { file_mode = "a"; }


    FILE* dump_file = fopen (file_path, file_mode);
    if (dump_file == nullptr) { LOG_ERROR (FILE_ERR); return; }


    setvbuf (dump_file, NULL, _IONBF, 0);


    fprintf (dump_file, "<pre><h2>");
    fprintf (dump_file,"%s", additional_text);
    fprintf (dump_file, "</h2>");
    fprintf (dump_file, "<h2>Dumping tree at %s in function %s (line %d)...</h2>\n\n", file, func, line);


    if (!tree) { fprintf (dump_file, "Tree pointer is nullptr!\n\n"); return; }


    fprintf (dump_file, "this tree has name ");
    if (tree->debug_info.name != nullptr) { fprintf (dump_file, "%s ", tree->debug_info.name); }
    else                                  { fprintf (dump_file, "UNKNOWN NAME "); }
    fprintf (dump_file, "[%p]\n", tree->debug_info.adress);

    fprintf (dump_file, "it was created in file ");
    if (tree->debug_info.birth_file != nullptr) { fprintf (dump_file, "%s\n", tree->debug_info.birth_file); }
    else                                        { fprintf (dump_file, "UNKNOWN NAME\n"); }

    fprintf (dump_file, "in function ");
    if (tree->debug_info.birth_func != nullptr) { fprintf (dump_file, "%s ", tree->debug_info.birth_func); }
    else                                        { fprintf (dump_file, "UNKNOWN NAME "); }

    fprintf (dump_file, "(line %d)\n\n", tree->debug_info.birth_line);


    fprintf (dump_file, "tree is ");
    Tree_state tree_state = tree_damaged (tree);
    if (tree_state) { fprintf (dump_file, "damaged (damage code %u)\n", tree_state); }
    else            { fprintf (dump_file, "ok\n"); }


    if (tree->root) { fprintf (dump_file, "root [%p].\n\n", tree->root); }
    else            { fprintf (dump_file, "root [nullptr].\n\n"); }



    fprintf (dump_file, "nodes:\n\n\n");

    fprintf (dump_file, "pre-order:\n\n");
    _fdump_nodes (dump_file, tree, "pre");

    fprintf (dump_file, "\n\nin-order:\n\n");
    _fdump_nodes (dump_file, tree, "in");


    fprintf (dump_file, "\n");


    char name [MAX_COMMAND_LEN] = tree_graph_file_name;
    strcat (name, GLOBAL_graph_dump_num);
    strcat (name, ".svg");
    fprintf (dump_file, "<img src=\"%s\" width=\"%zd\">", name, GRAPH_WIDTH);


    itoa ( (atoi (GLOBAL_graph_dump_num) + 1), GLOBAL_graph_dump_num, DEFAULT_COUNTING_SYSTEM); //incrementation of graph_dump_num


    fclose (dump_file);
}


void  tree_atexit_show_graph_sump  (void) {

    static bool first_time_calling = true;

    if (first_time_calling) { atexit (_tree_show_graph_dump); }

    first_time_calling = false;
}


void  _tree_show_graph_dump  (void) {

    char command [MAX_COMMAND_LEN] = "start ";
    strcat (command, tree_graph_dump_file_name);
    strcat (command, ".html");
    system (command);
}


void  _tree_generate_graph_describtion  (Tree* tree) {

    if (!tree) { return; }


    Tree_iterator tree_iterator = {};
    tree_iterator_ctor (&tree_iterator, tree, "pre");


    printf ("(tree): generating %s graph dump...\n", GLOBAL_graph_dump_num);


    char name [MAX_COMMAND_LEN] = "";
    strcat (name, tree_graph_describtion_file_name);
    strcat (name, GLOBAL_graph_dump_num);
    strcat (name, ".txt");

    FILE* graph_file = fopen (name, "w");
    if (graph_file == nullptr) { LOG_ERROR (FILE_ERR); return; }


    fprintf (graph_file, "digraph G {\n\n");


    fprintf (graph_file, "    bgcolor = \"#%s\"\n", tree_dump_bgclr);
    fprintf (graph_file, "    edge [minlen = 1, penwidth = 0.3, arrowsize = 0.3];\n    node [shape = record, style = rounded, fixedsize = true, height = 0.1, width = 0.6, fontsize = 3];\n\n");
    fprintf (graph_file, "    {rank = min; above_node [width = 3, style = invis];}\n\n");


    _tree_generate_nodes_describtion (tree, graph_file);


    fprintf (graph_file, "\n\n    {rank = max; below_node[width = 3, style = invis]; }\n\n");

    fprintf (graph_file, "    above_node -> node0_0 [style = invis]; below_node -> node%zd_0 [style = invis];\n\n", tree_depth (tree));


    fprintf (graph_file, "}");


    fclose (graph_file);
}


void  _tree_generate_nodes_describtion  (Tree* tree, FILE* file) {

    assert (tree && file);


    size_t given_tree_depth = tree_depth (tree);

    for (size_t depth = 0; depth <= given_tree_depth; depth++) {

        size_t on_level_index   = 0;
        size_t next_level_index = 0;

        Tree_iterator tree_iterator = {};
        tree_iterator_ctor (&tree_iterator, tree, "in");



        fprintf (file, "    {\n");

        do {

            if (tree_iterator.depth != depth) { continue; }


            if (_isleaf (tree_iterator.current)) { 

                fprintf (file, "        node%zd_%zd [style = \"rounded, filled\", color=\"#%s\", ", depth, on_level_index, tree_dump_leafclr);

                fprintf (file, "label = \"");

                switch (tree_iterator.current->atom_type) {

                    case DAT_OPERATION: fprintf (file, "%s",  _operation_code_to_str (tree_iterator.current->element.value.val_operation_code)); break;
                    case DAT_CONST:     fprintf (file, "%lf",                         tree_iterator.current->element.value.val_double);          break;
                    case DAT_VARIABLE:  fprintf (file, "%s",                          tree_iterator.current->element.value.var_str);             break;

                    default: LOG_ERROR (BAD_ARGS); return;
                }

                fprintf (file, "\"];\n\n");

                on_level_index += 1;
                continue;
            }


            fprintf (file, "        node%zd_%zd [style = \"rounded, filled\", color=\"#%s\", ", depth, on_level_index, tree_dump_nodeclr);
            fprintf (file, "label = \"");

            switch (tree_iterator.current->atom_type) {

                case DAT_OPERATION: fprintf (file, "%s",  _operation_code_to_str (tree_iterator.current->element.value.val_operation_code)); break;
                case DAT_CONST:     fprintf (file, "%lf",                         tree_iterator.current->element.value.val_double);          break;
                case DAT_VARIABLE:  fprintf (file, "%s",                          tree_iterator.current->element.value.var_str);             break;

                default: LOG_ERROR (BAD_ARGS); return;
            }

            fprintf (file, "\"]\n");


            fprintf (file, "        node%zd_%zd -> node%zd_%zd\n", depth, on_level_index, depth + 1, next_level_index);
            next_level_index += 1;

            fprintf (file, "        node%zd_%zd -> node%zd_%zd\n", depth, on_level_index, depth + 1, next_level_index);
            next_level_index += 1;


            on_level_index += 1;
        }

        while (!tree_iterator_inc (&tree_iterator));

        fprintf (file, "    }\n");
    }
}


void  _tree_generate_graph  (void) {

    char command [MAX_COMMAND_LEN] = "dot -Tsvg ";
    strcat (command, tree_graph_describtion_file_name);
    strcat (command, GLOBAL_graph_dump_num);
    strcat (command, ".txt");
    strcat (command, " -o ");
    strcat (command, "work/");
    strcat (command, tree_graph_file_name);
    strcat (command, GLOBAL_graph_dump_num);
    strcat (command, ".svg");
    system (command);
}


size_t  tree_depth  (Tree* tree) {

    size_t depth = 0;


    Tree_iterator tree_iterator = {};
    tree_iterator_ctor (&tree_iterator, tree, "in");


    do {

            if (tree_iterator.depth > depth) { depth = tree_iterator.depth; }
        }
        while (!tree_iterator_inc (&tree_iterator));


    return depth;
}


void _speak (const char* format, ...) {

    va_list args;


    va_start (args, format);


    char command    [MAX_COMMAND_LEN] = "";
    char new_format [MAX_COMMAND_LEN] = "";
    sprintf (new_format, "espeak -ven+whisper -p0 \" %s \" ", format);
    vsprintf (command, new_format, args);


    va_end (args);


    system  (command);
}


const char*  _operation_code_to_str  (Operation_code operation_code) {

    switch (operation_code) {

        case DOC_UNKNOWN: return "DOC_UNKNOWN";
        case DOC_ADD:  return "+";
        case DOC_SUB:  return "-";
        case DOC_MULT: return "*";
        case DOC_DIV:  return "/";

        default: return "UNKNOWN";
    }
}


//--------------------------------------------------


#endif