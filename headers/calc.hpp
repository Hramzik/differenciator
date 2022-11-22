#ifndef CALC_HPP_INCLUDED 
#define CALC_HPP_INCLUDED

//--------------------------------------------------
#include <assert.h>
#include <stdio.h>


int get_number  (const char** string_ptr, Node** node_ptr);
int get_primary (const char** string_ptr, Node** node_ptr);
int get_product (const char** string_ptr, Node** node_ptr);
int get_sum     (const char** string_ptr, Node** node_ptr);
int get_general (const char*  string, Tree* tree);


//--------------------------------------------------
#endif