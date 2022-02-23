#include "ec_list.h"
#include "ec_stack.h"

ect_list_define_undeclared(base_list_int_t, int, NULL);
ect_stack_define(stack_int_t, int, base_list_int_t);
