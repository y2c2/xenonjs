#ifndef TYPE_INTSTACK_H
#define TYPE_INTSTACK_H

#include "ec_list.h"
#include "ec_stack.h"

ect_list_declare(base_list_int_t, int);
ect_stack_declare(stack_int_t, int, base_list_int_t);

#endif
