/* XenonJS : AST
 * Copyright(c) 2017 y2c2 */

#include <ec_alloc.h>
#include <ec_string.h>
#include <ec_list.h>
#include <ec_enum.h>
#include "xjs_dt.h"
#include "xjs_alloc.h"
#include "xjs_helper.h"
#include "xjs_ast.h"

static void xjs_ast_loc_init(xjs_ast_loc_ref loc)
{
    loc->start.ln = -1;
    loc->start.col = -1;
    loc->end.ln = -1;
    loc->end.col = -1;
}


static void xjs_ast_range_init(xjs_ast_range_ref range)
{
    range->start = -1;
    range->end = -1;
}

/* AST : Identifier */

static void xjs_ast_identifier_ctor(void *data)
{
    xjs_ast_identifier_ref r = data; r->name = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range);
}

static void xjs_ast_identifier_dtor(void *data)
{ xjs_ast_identifier_ref r = data; ec_delete(r->name); }

xjs_ast_identifier_ref xjs_ast_identifier_new(ec_string *name)
{
    xjs_ast_identifier_ref r = ec_newcd( \
            xjs_ast_identifier, \
            xjs_ast_identifier_ctor, xjs_ast_identifier_dtor);
    r->name = name;
    return r;
}

/* AST : Identifier : Id */

static void xjs_ast_variabledeclarator_id_ctor(void *data)
{
    xjs_ast_variabledeclarator_id_ref r = data;
    r->tag = xjs_ast_variabledeclarator_id_identifier;
    r->u.xjs_ast_variabledecorator_identifier = NULL;
}

static void xjs_ast_variabledeclarator_id_dtor(void *data)
{
    xjs_ast_variabledeclarator_id_ref r = data;
    ec_delete(r->u.xjs_ast_variabledecorator_identifier);
}

xjs_ast_variabledeclarator_id_ref
xjs_ast_variabledeclarator_id_identifier_new(xjs_ast_identifier_ref id)
{
    xjs_ast_variabledeclarator_id_ref r = ec_newcd( \
            xjs_ast_variabledeclarator_id, \
            xjs_ast_variabledeclarator_id_ctor, \
            xjs_ast_variabledeclarator_id_dtor);
    r->u.xjs_ast_variabledecorator_identifier = id;
    return r;
}

/* AST : Literal */

static void xjs_ast_literal_ctor(void *data)
{
    xjs_ast_literal_ref r = data;
    r->raw = 0;
    r->type = xjs_ast_literal_unknown;
    xjs_ast_loc_init(&r->loc); 
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_literal_dtor(void *data)
{
    xjs_ast_literal_ref r = data;
    switch (r->type)
    {
        case xjs_ast_literal_unknown:
        case xjs_ast_literal_undefined:
        case xjs_ast_literal_null:
        case xjs_ast_literal_boolean:
        case xjs_ast_literal_number:
            break;
        case xjs_ast_literal_string:
            ec_delete(r->value.as_string);
            break;
    }
    ec_delete(r->raw);
}

xjs_ast_literal_ref xjs_ast_literal_new(void)
{
    xjs_ast_literal_ref r = ec_newcd(xjs_ast_literal, \
            xjs_ast_literal_ctor, \
            xjs_ast_literal_dtor);
    return r;
}

void xjs_ast_literal_value_set_undefined(xjs_ast_literal_ref r)
{
    r->type = xjs_ast_literal_undefined;
}

void xjs_ast_literal_value_set_null(xjs_ast_literal_ref r)
{
    r->type = xjs_ast_literal_null;
}

void xjs_ast_literal_value_set_boolean(xjs_ast_literal_ref r, ec_bool value)
{
    r->type = xjs_ast_literal_boolean;
    r->value.as_bool = value;
}

void xjs_ast_literal_value_set_number(xjs_ast_literal_ref r, double value)
{
    r->type = xjs_ast_literal_number;
    r->value.as_number = value;
}

void xjs_ast_literal_value_set_string(xjs_ast_literal_ref r, ec_string *value)
{
    r->type = xjs_ast_literal_string;
    r->value.as_string = value;
}

void xjs_ast_literal_raw_set(xjs_ast_literal_ref r, ec_string *raw)
{
    ec_delete(r->raw);
    r->raw = raw;
}

/* AST : Parameter */

static void xjs_ast_parameter_ctor(void *data)
{
    xjs_ast_parameter_ref r = data;
    r->id = NULL;
    xjs_ast_loc_init(&r->loc); 
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_parameter_dtor(void *data)
{
    xjs_ast_parameter_ref r = data;
    ec_delete(r->id);
}

xjs_ast_parameter_ref xjs_ast_parameter_new(xjs_ast_identifier_ref id)
{
    xjs_ast_parameter_ref r = ec_newcd(xjs_ast_parameter, \
            xjs_ast_parameter_ctor, xjs_ast_parameter_dtor);
    r->id = id;
    return r;
}

/* AST : ParameterList */

static void xjs_ast_parameterlist_node_dtor(xjs_ast_parameter_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ast_parameterlist, xjs_ast_parameter_ref, xjs_ast_parameterlist_node_dtor);

/* AST : Property */

static void xjs_ast_property_ctor(void *data)
{
    xjs_ast_property_ref r = data;
    r->key = NULL;
    r->value = NULL;
    xjs_ast_loc_init(&r->loc); 
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_property_dtor(void *data)
{
    xjs_ast_property_ref r = data;
    ec_delete(r->key);
    ec_delete(r->value);
}

xjs_ast_property_ref xjs_ast_property_new(void)
{
    xjs_ast_property_ref r = ec_newcd(xjs_ast_property, \
            xjs_ast_property_ctor, xjs_ast_property_dtor);
    return r;
}

void xjs_ast_property_set_key(xjs_ast_property_ref property, xjs_ast_expression_ref key)
{
    property->key = key;
}

void xjs_ast_property_set_value(xjs_ast_property_ref property, xjs_ast_expression_ref value)
{
    property->value = value;
}

static void xjs_ast_propertylist_node_dtor(xjs_ast_property_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ast_propertylist, xjs_ast_property_ref, xjs_ast_propertylist_node_dtor);

/* AST : VariableDeclarator */

static void xjs_ast_variabledeclarator_ctor(void *data)
{
    xjs_ast_variabledeclarator_ref r = data;
    r->id = NULL;
    r->init = NULL;
}

static void xjs_ast_variabledeclarator_dtor(void *data)
{
    xjs_ast_variabledeclarator_ref r = data;
    ec_delete(r->id);
    ec_delete(r->init);
}

xjs_ast_variabledeclarator_ref xjs_ast_variabledeclarator_new(void)
{
    xjs_ast_variabledeclarator_ref r = ec_newcd( \
            xjs_ast_variabledeclarator, \
            xjs_ast_variabledeclarator_ctor, \
            xjs_ast_variabledeclarator_dtor);
    return r;
}

void xjs_ast_variabledeclarator_id_set( \
        xjs_ast_variabledeclarator_ref r, \
        xjs_ast_variabledeclarator_id_ref decl_id)
{
    ec_delete(r->id);
    r->id = decl_id;
}

xjs_ast_variabledeclarator_id_ref xjs_ast_variabledeclarator_id_get( \
        xjs_ast_variabledeclarator_ref r)
{
    return r->id;
}

void xjs_ast_variabledeclarator_init_set( \
        xjs_ast_variabledeclarator_ref r, \
        xjs_ast_expression_ref expr)
{
    ec_delete(r->init);
    r->init = expr;
}

xjs_ast_expression_ref xjs_ast_variabledeclarator_init_get( \
        xjs_ast_variabledeclarator_ref r)
{
    return r->init;
}

/* AST : VariableDeclaratorList */

static void xjs_ast_variabledeclaratorlist_node_dtor(xjs_ast_variabledeclarator_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ast_variabledeclaratorlist, \
        xjs_ast_variabledeclarator_ref, \
        xjs_ast_variabledeclaratorlist_node_dtor);

/* AST : VariableDeclaration */

static void xjs_ast_variabledeclaration_ctor(void *data)
{
    xjs_ast_variabledeclaration_ref r = data;
    r->kind = xjs_ast_variabledeclaration_kind_var;
    r->declarations = xjs_ast_variabledeclaratorlist_new();
}

static void xjs_ast_variabledeclaration_dtor(void *data)
{
    xjs_ast_variabledeclaration_ref r = data;
    ec_delete(r->declarations);
}

xjs_ast_variabledeclaration_ref xjs_ast_variabledeclaration_new(void) 
{
    xjs_ast_variabledeclaration_ref r = ec_newcd(xjs_ast_variabledeclaration, \
            xjs_ast_variabledeclaration_ctor, \
            xjs_ast_variabledeclaration_dtor);
    return r;
}

void xjs_ast_variabledeclaration_kind_set( \
        xjs_ast_variabledeclaration_ref r, \
        xjs_ast_variabledeclaration_kind kind)
{
    r->kind = kind;
}

xjs_ast_variabledeclaration_kind
xjs_ast_variabledeclaration_kind_get( \
        xjs_ast_variabledeclaration_ref r)
{
    return r->kind;
}


/* AST : UpdateExpression */

static void xjs_ast_updateexpression_ctor(void *data)
{
    xjs_ast_updateexpression_ref r = data;
    r->op = NULL;
    r->argument = NULL;
    r->prefix = ec_false;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_updateexpression_dtor(void *data)
{
    xjs_ast_updateexpression_ref r = data;
    ec_delete(r->op);
    ec_delete(r->argument);
}

xjs_ast_updateexpression_ref xjs_ast_updateexpression_new(void)
{
    xjs_ast_updateexpression_ref r = ec_newcd(xjs_ast_updateexpression, \
            xjs_ast_updateexpression_ctor, \
            xjs_ast_updateexpression_dtor);
    return r;
}

void xjs_ast_updateexpression_op_set( \
        xjs_ast_updateexpression_ref r, \
        ec_string *op)
{
    ec_delete(r->op);
    r->op = op;
}

void xjs_ast_updateexpression_argument_set( \
        xjs_ast_updateexpression_ref r, \
        xjs_ast_expression_ref argument)
{
    ec_delete(r->argument);
    r->argument = argument;
}

void xjs_ast_updateexpression_prefix_set( \
        xjs_ast_updateexpression_ref r, \
        ec_bool prefix)
{
    r->prefix = prefix;
}

/* AST : UnaryExpression */

static void xjs_ast_unaryexpression_ctor(void *data)
{
    xjs_ast_unaryexpression_ref r = data;
    r->op = NULL;
    r->argument = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_unaryexpression_dtor(void *data)
{
    xjs_ast_unaryexpression_ref r = data;
    ec_delete(r->op);
    ec_delete(r->argument);
}

xjs_ast_unaryexpression_ref xjs_ast_unaryexpression_new(void)
{
    xjs_ast_unaryexpression_ref r = ec_newcd(xjs_ast_unaryexpression, \
            xjs_ast_unaryexpression_ctor, \
            xjs_ast_unaryexpression_dtor);
    return r;
}

void xjs_ast_unaryexpression_op_set( \
        xjs_ast_unaryexpression_ref r, \
        ec_string *op)
{
    ec_delete(r->op);
    r->op = op;
}

void xjs_ast_unaryexpression_argument_set( \
        xjs_ast_unaryexpression_ref r, \
        xjs_ast_expression_ref argument)
{
    ec_delete(r->argument);
    r->argument = argument;
}

/* AST : AssignmentExpression */

static void xjs_ast_assignmentexpression_ctor(void *data)
{
    xjs_ast_assignmentexpression_ref r = data;
    r->op = NULL;
    r->left = NULL;
    r->right = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_assignmentexpression_dtor(void *data)
{
    xjs_ast_assignmentexpression_ref r = data;
    ec_delete(r->op);
    ec_delete(r->left);
    ec_delete(r->right);
}

xjs_ast_assignmentexpression_ref xjs_ast_assignmentexpression_new(void)
{
    xjs_ast_assignmentexpression_ref r = ec_newcd(xjs_ast_assignmentexpression, \
            xjs_ast_assignmentexpression_ctor, \
            xjs_ast_assignmentexpression_dtor);
    return r;
}

void xjs_ast_assignmentexpression_op_set( \
        xjs_ast_assignmentexpression_ref r, \
        ec_string *op)
{
    r->op = op;
}

void xjs_ast_assignmentexpression_left_set( \
        xjs_ast_assignmentexpression_ref r, \
        xjs_ast_expression_ref left)
{
    r->left = left;
}

void xjs_ast_assignmentexpression_right_set( \
        xjs_ast_assignmentexpression_ref r, \
        xjs_ast_expression_ref right)
{
    r->right = right;
}

/* AST : BinaryExpression */

static void xjs_ast_binaryexpression_ctor(void *data)
{
    xjs_ast_binaryexpression_ref r = data;
    r->op = NULL;
    r->left = r->right = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_binaryexpression_dtor(void *data)
{
    xjs_ast_binaryexpression_ref r = data;
    ec_delete(r->op);
    ec_delete(r->left);
    ec_delete(r->right);
}

xjs_ast_binaryexpression_ref xjs_ast_binaryexpression_new(void)
{
    xjs_ast_binaryexpression_ref r = ec_newcd(xjs_ast_binaryexpression, \
            xjs_ast_binaryexpression_ctor, \
            xjs_ast_binaryexpression_dtor);
    return r;
}

void xjs_ast_binaryexpression_op_set( \
        xjs_ast_binaryexpression_ref r, \
        ec_string *op)
{
    ec_delete(r->op);
    r->op = op;
}

void xjs_ast_binaryexpression_left_set( \
        xjs_ast_binaryexpression_ref r, \
        xjs_ast_expression_ref left)
{
    ec_delete(r->left);
    r->left = left;
}

void xjs_ast_binaryexpression_right_set( \
        xjs_ast_binaryexpression_ref r, \
        xjs_ast_expression_ref right)
{
    ec_delete(r->right);
    r->right = right;
}

/* AST : LogicalExpression */

static void xjs_ast_logicalexpression_ctor(void *data)
{
    xjs_ast_logicalexpression_ref r = data;
    r->op = NULL;
    r->left = r->right = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_logicalexpression_dtor(void *data)
{
    xjs_ast_logicalexpression_ref r = data;
    ec_delete(r->op);
    ec_delete(r->left);
    ec_delete(r->right);
}

xjs_ast_logicalexpression_ref xjs_ast_logicalexpression_new(void)
{
    xjs_ast_logicalexpression_ref r = ec_newcd(xjs_ast_logicalexpression, \
            xjs_ast_logicalexpression_ctor, \
            xjs_ast_logicalexpression_dtor);
    return r;
}

void xjs_ast_logicalexpression_op_set( \
        xjs_ast_logicalexpression_ref r, \
        ec_string *op)
{
    r->op = op;
}

void xjs_ast_logicalexpression_left_set( \
        xjs_ast_logicalexpression_ref r, \
        xjs_ast_expression_ref left)
{
    r->left = left;
}

void xjs_ast_logicalexpression_right_set( \
        xjs_ast_logicalexpression_ref r, \
        xjs_ast_expression_ref right)
{
    r->right = right;
}

/* AST : FunctionExpression */

static void xjs_ast_functionexpression_ctor(void *data)
{
    xjs_ast_functionexpression_ref r = data;
    r->id = NULL;
    r->params = xjs_ast_parameterlist_new();
    r->body = xjs_ast_statementlist_new();
    xjs_ast_loc_init(&r->loc); 
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_functionexpression_dtor(void *data)
{
    xjs_ast_functionexpression_ref r = data;
    ec_delete(r->id);
    ec_delete(r->params);
    ec_delete(r->body);
}

xjs_ast_functionexpression_ref xjs_ast_functionexpression_new(void)
{
    xjs_ast_functionexpression_ref r = ec_newcd(xjs_ast_functionexpression, \
            xjs_ast_functionexpression_ctor, xjs_ast_functionexpression_dtor);
    return r;
}

void xjs_ast_functionexpression_id_set( \
        xjs_ast_functionexpression_ref r, \
        ec_string *id)
{
    ec_delete(r->id);
    r->id = id;
}

int xjs_ast_functionexpression_parameter_push_back( \
        xjs_ast_functionexpression_ref r, \
        xjs_ast_identifier_ref id)
{
    xjs_ast_parameter_ref new_parameter;

    if ((new_parameter = xjs_ast_parameter_new(id)) == NULL)
    { return -1; }

    xjs_ast_parameterlist_push_back(r->params, new_parameter);
    return 0;
}

void xjs_ast_functionexpression_body_set( \
        xjs_ast_functionexpression_ref r, \
        xjs_ast_statementlistitem_ref body)
{
    ec_delete(r->body);
    r->body = body;
}

/* AST : ArrowFunctionExpression */

static void xjs_ast_arrowfunctionexpression_ctor(void *data)
{
    xjs_ast_arrowfunctionexpression_ref r = data;
    r->params = xjs_ast_parameterlist_new();
    r->body.type = xjs_opaque_ast_arrowfunctionexpression_body_expr;
    r->body.u.as_expr = NULL;
    xjs_ast_loc_init(&r->loc); 
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_arrowfunctionexpression_dtor(void *data)
{
    xjs_ast_arrowfunctionexpression_ref r = data;
    ec_delete(r->params);
    switch (r->body.type)
    {
        case xjs_opaque_ast_arrowfunctionexpression_body_blockstmt:
            ec_delete(r->body.u.as_blockstmt);
            break;
        case xjs_opaque_ast_arrowfunctionexpression_body_expr:
            ec_delete(r->body.u.as_expr);
            break;
    }
}

xjs_ast_arrowfunctionexpression_ref xjs_ast_arrowfunctionexpression_new(void)
{
    xjs_ast_arrowfunctionexpression_ref r = ec_newcd(xjs_ast_arrowfunctionexpression, \
            xjs_ast_arrowfunctionexpression_ctor, \
            xjs_ast_arrowfunctionexpression_dtor);
    return r;
}

int xjs_ast_arrowfunctionexpression_parameter_push_back( \
        xjs_ast_arrowfunctionexpression_ref r, \
        xjs_ast_identifier_ref id)
{
    xjs_ast_parameter_ref new_parameter;

    if ((new_parameter = xjs_ast_parameter_new(id)) == NULL)
    { return -1; }

    xjs_ast_parameterlist_push_back(r->params, new_parameter);
    return 0;
}

void xjs_ast_arrowfunctionexpression_body_expr_set( \
        xjs_ast_arrowfunctionexpression_ref r, \
        xjs_ast_expression_ref expr)
{
    r->body.type = xjs_opaque_ast_arrowfunctionexpression_body_expr;
    r->body.u.as_expr = expr;
}

void xjs_ast_arrowfunctionexpression_body_statement_set( \
        xjs_ast_arrowfunctionexpression_ref r, \
        xjs_ast_statementlistitem_ref blockstmt)
{
    r->body.type = xjs_opaque_ast_arrowfunctionexpression_body_blockstmt;
    r->body.u.as_blockstmt = blockstmt;
}

/* AST : ObjectExpression */

static void xjs_ast_objectexpression_ctor(void *data)
{
    xjs_ast_objectexpression_ref r = data;
    r->properties = ect_list_new(xjs_ast_propertylist);
    xjs_ast_loc_init(&r->loc); 
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_objectexpression_dtor(void *data)
{
    xjs_ast_objectexpression_ref r = data;
    ec_delete(r->properties);
}

xjs_ast_objectexpression_ref xjs_ast_objectexpression_new(void)
{
    xjs_ast_objectexpression_ref r = ec_newcd(xjs_ast_objectexpression, \
            xjs_ast_objectexpression_ctor, xjs_ast_objectexpression_dtor);
    return r;
}

int xjs_ast_objectexpression_property_push_back( \
        xjs_ast_objectexpression_ref r, \
        xjs_ast_property_ref p)
{
    ect_list_push_back(xjs_ast_propertylist, r->properties, p);
    return 0;
}

/* AST : ArrayExpression */

static void xjs_ast_arrayexpression_ctor(void *data)
{
    xjs_ast_arrayexpression_ref r = data;
    r->elements = ect_list_new(xjs_ast_expressionlist);
    xjs_ast_loc_init(&r->loc); 
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_arrayexpression_dtor(void *data)
{
    xjs_ast_arrayexpression_ref r = data;
    ec_delete(r->elements);
}

xjs_ast_arrayexpression_ref xjs_ast_arrayexpression_new(void)
{
    xjs_ast_arrayexpression_ref r = ec_newcd(xjs_ast_arrayexpression, \
            xjs_ast_arrayexpression_ctor, xjs_ast_arrayexpression_dtor);
    return r;
}

int xjs_ast_arrayexpression_element_push_back( \
        xjs_ast_arrayexpression_ref r, \
        xjs_ast_expression_ref expr)
{
    ect_list_push_back(xjs_ast_expressionlist, r->elements, expr);
    return 0;
}

/* AST : CallExpression */

static void xjs_ast_callexpression_ctor(void *data)
{
    xjs_ast_callexpression_ref r = data;
    r->callee = NULL;
    r->arguments = NULL;
    xjs_ast_loc_init(&r->loc); 
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_callexpression_dtor(void *data)
{
    xjs_ast_callexpression_ref r = data;
    ec_delete(r->callee);
    ec_delete(r->arguments);
}

xjs_ast_callexpression_ref xjs_ast_callexpression_new(void)
{
    xjs_ast_callexpression_ref r = ec_newcd(xjs_ast_callexpression, \
            xjs_ast_callexpression_ctor, xjs_ast_callexpression_dtor);
    return r;
}

void xjs_ast_callexpression_callee_set( \
        xjs_ast_callexpression_ref r, \
        xjs_ast_expression_ref callee)
{
    r->callee = callee;
}

void xjs_ast_callexpression_arguments_set( \
        xjs_ast_callexpression_ref r, \
        xjs_ast_expressionlist_ref arguments)
{
    r->arguments = arguments;
}

/* AST : MemberExpression */

static void xjs_ast_memberexpression_ctor(void *data)
{
    xjs_ast_memberexpression_ref r = data;
    r->computed = ec_false;
    r->object = NULL;
    r->property = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_memberexpression_dtor(void *data)
{
    xjs_ast_memberexpression_ref r = data;
    ec_delete(r->object);
    ec_delete(r->property);
}

xjs_ast_memberexpression_ref xjs_ast_memberexpression_new(void)
{
    xjs_ast_memberexpression_ref r = ec_newcd(xjs_ast_memberexpression, \
            xjs_ast_memberexpression_ctor, xjs_ast_memberexpression_dtor);
    return r;
}

void xjs_ast_memberexpression_computed_set( \
        xjs_ast_memberexpression_ref r, \
        ec_bool computed)
{
    r->computed = computed;
}

void xjs_ast_memberexpression_object_set( \
        xjs_ast_memberexpression_ref r, \
        xjs_ast_expression_ref object)
{
    r->object = object;
}

void xjs_ast_memberexpression_property_set( \
        xjs_ast_memberexpression_ref r, \
        xjs_ast_expression_ref prop)
{
    r->property = prop;
}

/* AST : NewExpression */

static void xjs_ast_newexpression_ctor(void *data)
{
    xjs_ast_newexpression_ref r = data;
    r->callee = NULL;
    r->arguments = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_newexpression_dtor(void *data)
{
    xjs_ast_newexpression_ref r = data;
    ec_delete(r->callee);
    ec_delete(r->arguments);
}

xjs_ast_newexpression_ref xjs_ast_newexpression_new(void)
{
    xjs_ast_newexpression_ref r = ec_newcd(xjs_ast_newexpression, \
            xjs_ast_newexpression_ctor, xjs_ast_newexpression_dtor);
    return r;
}

void xjs_ast_newexpression_callee_set( \
        xjs_ast_newexpression_ref r, \
        xjs_ast_expression_ref callee)
{
    r->callee = callee;
}

void xjs_ast_newexpression_arguments_set( \
        xjs_ast_newexpression_ref r, \
        xjs_ast_expressionlist_ref arguments)
{
    r->arguments = arguments;
}

/* AST : ConditionalExpression */

static void xjs_ast_conditionalexpression_ctor(void *data)
{
    xjs_ast_conditionalexpression_ref r = data;
    r->test = NULL;
    r->consequent = NULL;
    r->alternate = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_conditionalexpression_dtor(void *data)
{
    xjs_ast_conditionalexpression_ref r = data;
    ec_delete(r->test);
    ec_delete(r->consequent);
    ec_delete(r->alternate);
}

xjs_ast_conditionalexpression_ref xjs_ast_conditionalexpression_new(void)
{
    xjs_ast_conditionalexpression_ref r = ec_newcd(xjs_ast_conditionalexpression, \
            xjs_ast_conditionalexpression_ctor, xjs_ast_conditionalexpression_dtor);
    return r;
}

void xjs_ast_conditionalexpression_test_set( \
        xjs_ast_conditionalexpression_ref r, \
        xjs_ast_expression_ref test)
{
    r->test = test;
}

void xjs_ast_conditionalexpression_consequent_set( \
        xjs_ast_conditionalexpression_ref r, \
        xjs_ast_expression_ref consequent)
{
    r->consequent = consequent;
}

void xjs_ast_conditionalexpression_alternate_set( \
        xjs_ast_conditionalexpression_ref r, \
        xjs_ast_expression_ref alternate)
{
    r->alternate = alternate;
}

/* AST : ThisExpression */

static void xjs_ast_thisexpression_ctor(void *data)
{
    xjs_ast_thisexpression_ref r = data;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_thisexpression_dtor(void *data)
{
    xjs_ast_thisexpression_ref r = data;
    (void)r;
}

xjs_ast_thisexpression_ref xjs_ast_thisexpression_new(void)
{
    xjs_ast_thisexpression_ref r = ec_newcd(xjs_ast_thisexpression, \
            xjs_ast_thisexpression_ctor, xjs_ast_thisexpression_dtor);
    return r;
}

/* AST : Expression */

static void xjs_ast_expression_identifier_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_identifier;
    r->u.xjs_ast_expression_identifier = NULL;
}

static void xjs_ast_expression_identifier_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_identifier);
}

xjs_ast_expression_ref xjs_ast_expression_identifier_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_identifier_ctor,
            xjs_ast_expression_identifier_dtor);
    return r;

}

void xjs_ast_expression_identifier_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_identifier_ref id)
{
    ec_delete(r->u.xjs_ast_expression_identifier);
    r->u.xjs_ast_expression_identifier = id;
}

static void xjs_ast_expression_literal_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_literal;
    r->u.xjs_ast_expression_literal = NULL;
}

static void xjs_ast_expression_literal_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_literal);
}

xjs_ast_expression_ref xjs_ast_expression_literal_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_literal_ctor,
            xjs_ast_expression_literal_dtor);
    return r;
}

void xjs_ast_expression_literal_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_literal_ref lit)
{
    ec_delete(r->u.xjs_ast_expression_literal);
    r->u.xjs_ast_expression_literal = lit;
}

static void xjs_ast_expression_unary_expression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_unaryexpression;
    r->u.xjs_ast_expression_unaryexpression = NULL;
}

static void xjs_ast_expression_unary_expression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_unaryexpression);
}

xjs_ast_expression_ref xjs_ast_expression_unaryexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_unary_expression_ctor, \
            xjs_ast_expression_unary_expression_dtor);
    return r;
}

void xjs_ast_expression_unaryexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_unaryexpression_ref unary_expr)
{
    ec_delete(r->u.xjs_ast_expression_unaryexpression);
    r->u.xjs_ast_expression_unaryexpression = unary_expr;
}

static void xjs_ast_expression_binary_expression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_binaryexpression;
    r->u.xjs_ast_expression_binaryexpression = NULL;
}

static void xjs_ast_expression_binary_expression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_binaryexpression);
}

xjs_ast_expression_ref xjs_ast_expression_binaryexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_binary_expression_ctor, \
            xjs_ast_expression_binary_expression_dtor);
    return r;
}

void xjs_ast_expression_binaryexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_binaryexpression_ref binary_expr)
{
    ec_delete(r->u.xjs_ast_expression_binaryexpression);
    r->u.xjs_ast_expression_binaryexpression = binary_expr;
}

static void xjs_ast_expression_logical_expression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_logicalexpression;
    r->u.xjs_ast_expression_logicalexpression = NULL;
}

static void xjs_ast_expression_logical_expression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_logicalexpression);
}

xjs_ast_expression_ref xjs_ast_expression_logicalexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_logical_expression_ctor, \
            xjs_ast_expression_logical_expression_dtor);
    return r;
}

void xjs_ast_expression_logicalexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_logicalexpression_ref logical_expr)
{
    r->u.xjs_ast_expression_logicalexpression = logical_expr;
}

static void xjs_ast_expression_assignment_expression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_assignmentexpression;
    r->u.xjs_ast_expression_assignmentexpression = NULL;
}

static void xjs_ast_expression_assignment_expression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_assignmentexpression);
}

xjs_ast_expression_ref xjs_ast_expression_assignmentexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_assignment_expression_ctor, \
            xjs_ast_expression_assignment_expression_dtor);
    return r;
}

void xjs_ast_expression_assignmentexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_assignmentexpression_ref assignment_expr)
{
    ec_delete(r->u.xjs_ast_expression_assignmentexpression);
    r->u.xjs_ast_expression_assignmentexpression = assignment_expr;
}

static void xjs_ast_expression_updateexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_updateexpression;
    r->u.xjs_ast_expression_updateexpression = NULL;
}

static void xjs_ast_expression_updateexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_updateexpression);
}

xjs_ast_expression_ref xjs_ast_expression_updateexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_updateexpression_ctor, \
            xjs_ast_expression_updateexpression_dtor);
    return r;
}

void xjs_ast_expression_updateexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_updateexpression_ref update_expr)
{
    ec_delete(r->u.xjs_ast_expression_updateexpression);
    r->u.xjs_ast_expression_updateexpression = update_expr;
}

static void xjs_ast_expression_functionexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_functionexpression;
    r->u.xjs_ast_expression_functionexpression = NULL;
}

static void xjs_ast_expression_functionexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_functionexpression);
}

xjs_ast_expression_ref xjs_ast_expression_functionexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_functionexpression_ctor, \
            xjs_ast_expression_functionexpression_dtor);
    return r;
}

void xjs_ast_expression_functionexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_functionexpression_ref function_expr)
{
    r->u.xjs_ast_expression_functionexpression = function_expr;
}

static void xjs_ast_expression_arrowfunctionexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_arrowfunctionexpression;
    r->u.xjs_ast_expression_arrowfunctionexpression = NULL;
}

static void xjs_ast_expression_arrowfunctionexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_arrowfunctionexpression);
}

xjs_ast_expression_ref xjs_ast_expression_arrowfunctionexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_arrowfunctionexpression_ctor, \
            xjs_ast_expression_arrowfunctionexpression_dtor);
    return r;
}

void xjs_ast_expression_arrowfunctionexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_arrowfunctionexpression_ref arrowfunction_expr)
{
    r->u.xjs_ast_expression_arrowfunctionexpression = arrowfunction_expr;
}

static void xjs_ast_expression_objectexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_objectexpression;
    r->u.xjs_ast_expression_objectexpression = NULL;
}

static void xjs_ast_expression_objectexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_objectexpression);
}

xjs_ast_expression_ref xjs_ast_expression_objectexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_objectexpression_ctor, \
            xjs_ast_expression_objectexpression_dtor);
    return r;
}

void xjs_ast_expression_objectexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_objectexpression_ref object_expr)
{
    r->u.xjs_ast_expression_objectexpression = object_expr;
}

static void xjs_ast_expression_arrayexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_arrayexpression;
    r->u.xjs_ast_expression_arrayexpression = NULL;
}

static void xjs_ast_expression_arrayexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_arrayexpression);
}

xjs_ast_expression_ref xjs_ast_expression_arrayexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_arrayexpression_ctor, \
            xjs_ast_expression_arrayexpression_dtor);
    return r;
}

void xjs_ast_expression_arrayexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_arrayexpression_ref array_expr)
{
    r->u.xjs_ast_expression_arrayexpression = array_expr;
}

static void xjs_ast_expression_callexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_callexpression;
    r->u.xjs_ast_expression_callexpression = NULL;
}

static void xjs_ast_expression_callexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_callexpression);
}

xjs_ast_expression_ref xjs_ast_expression_callexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_callexpression_ctor, \
            xjs_ast_expression_callexpression_dtor);
    return r;
}

void xjs_ast_expression_callexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_callexpression_ref call_expr)
{
    r->u.xjs_ast_expression_callexpression = call_expr;
}

static void xjs_ast_expression_memberexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_memberexpression;
    r->u.xjs_ast_expression_memberexpression = NULL;
}

static void xjs_ast_expression_memberexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_memberexpression);
}

xjs_ast_expression_ref xjs_ast_expression_memberexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_memberexpression_ctor, \
            xjs_ast_expression_memberexpression_dtor);
    return r;
}

void xjs_ast_expression_memberexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_memberexpression_ref member_expr)
{
    r->u.xjs_ast_expression_memberexpression = member_expr;
}

static void xjs_ast_expression_newexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_newexpression;
    r->u.xjs_ast_expression_newexpression = NULL;
}

static void xjs_ast_expression_newexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_newexpression);
}

xjs_ast_expression_ref xjs_ast_expression_newexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_newexpression_ctor, \
            xjs_ast_expression_newexpression_dtor);
    return r;
}

void xjs_ast_expression_newexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_newexpression_ref new_expr)
{
    r->u.xjs_ast_expression_newexpression = new_expr;
}

static void xjs_ast_expression_conditionalexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_conditionalexpression;
    r->u.xjs_ast_expression_conditionalexpression = NULL;
}

static void xjs_ast_expression_conditionalexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_conditionalexpression);
}

xjs_ast_expression_ref xjs_ast_expression_conditionalexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_conditionalexpression_ctor, \
            xjs_ast_expression_conditionalexpression_dtor);
    return r;
}

void xjs_ast_expression_conditionalexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_conditionalexpression_ref conditional_expr)
{
    r->u.xjs_ast_expression_conditionalexpression = conditional_expr;
}

static void xjs_ast_expression_thisexpression_ctor(void *data)
{
    xjs_ast_expression_ref r = data;
    r->tag = xjs_ast_expression_thisexpression;
    r->u.xjs_ast_expression_thisexpression = NULL;
}

static void xjs_ast_expression_thisexpression_dtor(void *data)
{
    xjs_ast_expression_ref r = data;
    ec_delete(r->u.xjs_ast_expression_thisexpression);
}

xjs_ast_expression_ref xjs_ast_expression_thisexpression_new(void)
{
    xjs_ast_expression_ref r = ec_newcd(xjs_ast_expression, \
            xjs_ast_expression_thisexpression_ctor, \
            xjs_ast_expression_thisexpression_dtor);
    return r;
}

void xjs_ast_expression_thisexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_thisexpression_ref this_expr)
{
    r->u.xjs_ast_expression_thisexpression = this_expr;
}

/* AST : Empty Statement */

static void xjs_ast_emptystatement_ctor(void *data)
{
    xjs_ast_emptystatement_ref r = data;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_emptystatement_dtor(void *data)
{
    xjs_ast_emptystatement_ref r = data;
    (void)r;
}

xjs_ast_emptystatement_ref xjs_ast_emptystatement_new(void)
{
    xjs_ast_emptystatement_ref r = ec_newcd(xjs_ast_emptystatement, \
            xjs_ast_emptystatement_ctor, \
            xjs_ast_emptystatement_dtor);
    return r;
}

/* AST : Expression Statement */

static void xjs_ast_expressionstatement_ctor(void *data)
{
    xjs_ast_expressionstatement_ref r = data;
    r->expression = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_expressionstatement_dtor(void *data)
{
    xjs_ast_expressionstatement_ref r = data;
    ec_delete(r->expression);
}

xjs_ast_expressionstatement_ref xjs_ast_expressionstatement_new(void)
{
    xjs_ast_expressionstatement_ref r = ec_newcd(xjs_ast_expressionstatement, \
            xjs_ast_expressionstatement_ctor, \
            xjs_ast_expressionstatement_dtor);
    return r;
}

/* AST : Block Statement */

static void xjs_ast_blockstatement_ctor(void *data)
{
    xjs_ast_blockstatement_ref r = data;
    r->body = xjs_ast_statementlist_new();
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_blockstatement_dtor(void *data)
{
    xjs_ast_blockstatement_ref r = data;
    ec_delete(r->body);
}

xjs_ast_blockstatement_ref xjs_ast_blockstatement_new(void)
{
    xjs_ast_blockstatement_ref r = ec_newcd(xjs_ast_blockstatement, \
            xjs_ast_blockstatement_ctor, \
            xjs_ast_blockstatement_dtor);
    return r;
}

/* AST : ExpressionList */

static void xjs_ast_expressionlist_node_dtor(xjs_ast_expression_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ast_expressionlist, xjs_ast_expression_ref, xjs_ast_expressionlist_node_dtor);

/* AST : Inspect Statement */

static void xjs_ast_inspectstatement_ctor(void *data)
{
    xjs_ast_inspectstatement_ref r = data;
    r->argument = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_inspectstatement_dtor(void *data)
{
    xjs_ast_inspectstatement_ref r = data;
    ec_delete(r->argument);
}

xjs_ast_inspectstatement_ref xjs_ast_inspectstatement_new(void)
{
    xjs_ast_inspectstatement_ref r = ec_newcd(xjs_ast_inspectstatement, \
            xjs_ast_inspectstatement_ctor, \
            xjs_ast_inspectstatement_dtor);
    return r;
}

/* AST : Return Statement */

static void xjs_ast_returnstatement_ctor(void *data)
{
    xjs_ast_returnstatement_ref r = data;
    r->argument = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_returnstatement_dtor(void *data)
{
    xjs_ast_returnstatement_ref r = data;
    ec_delete(r->argument);
}

xjs_ast_returnstatement_ref xjs_ast_returnstatement_new(void)
{
    xjs_ast_returnstatement_ref r = ec_newcd(xjs_ast_returnstatement, \
            xjs_ast_returnstatement_ctor, \
            xjs_ast_returnstatement_dtor);
    return r;
}

/* AST : Break Statement */

static void xjs_ast_breakstatement_ctor(void *data)
{
    xjs_ast_breakstatement_ref r = data;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_breakstatement_dtor(void *data)
{
    xjs_ast_breakstatement_ref r = data;
    (void)r;
}

xjs_ast_breakstatement_ref xjs_ast_breakstatement_new(void)
{
    xjs_ast_breakstatement_ref r = ec_newcd(xjs_ast_breakstatement, \
            xjs_ast_breakstatement_ctor, \
            xjs_ast_breakstatement_dtor);
    return r;
}

/* AST : Continue Statement */

static void xjs_ast_continuestatement_ctor(void *data)
{
    xjs_ast_continuestatement_ref r = data;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_continuestatement_dtor(void *data)
{
    xjs_ast_continuestatement_ref r = data;
    (void)r;
}

xjs_ast_continuestatement_ref xjs_ast_continuestatement_new(void)
{
    xjs_ast_continuestatement_ref r = ec_newcd(xjs_ast_continuestatement, \
            xjs_ast_continuestatement_ctor, \
            xjs_ast_continuestatement_dtor);
    return r;
}

/* AST : If Statement */

static void xjs_ast_ifstatement_ctor(void *data)
{
    xjs_ast_ifstatement_ref r = data;
    r->test = NULL;
    r->consequent = NULL;
    r->alternate = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_ifstatement_dtor(void *data)
{
    xjs_ast_ifstatement_ref r = data;
    ec_delete(r->test);
    ec_delete(r->consequent);
    ec_delete(r->alternate);
}

xjs_ast_ifstatement_ref xjs_ast_ifstatement_new(void)
{
    xjs_ast_ifstatement_ref r = ec_newcd(xjs_ast_ifstatement, \
            xjs_ast_ifstatement_ctor, \
            xjs_ast_ifstatement_dtor);
    return r;
}

/* AST : While Statement */

static void xjs_ast_whilestatement_ctor(void *data)
{
    xjs_ast_whilestatement_ref r = data;
    r->test = NULL;
    r->body = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_whilestatement_dtor(void *data)
{
    xjs_ast_whilestatement_ref r = data;
    ec_delete(r->test);
    ec_delete(r->body);
}

xjs_ast_whilestatement_ref xjs_ast_whilestatement_new(void)
{
    xjs_ast_whilestatement_ref r = ec_newcd(xjs_ast_whilestatement, \
            xjs_ast_whilestatement_ctor, \
            xjs_ast_whilestatement_dtor);
    return r;
}

/* AST : Do Statement */

static void xjs_ast_dostatement_ctor(void *data)
{
    xjs_ast_dostatement_ref r = data;
    r->test = NULL;
    r->body = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_dostatement_dtor(void *data)
{
    xjs_ast_dostatement_ref r = data;
    ec_delete(r->test);
    ec_delete(r->body);
}

xjs_ast_dostatement_ref xjs_ast_dostatement_new(void)
{
    xjs_ast_dostatement_ref r = ec_newcd(xjs_ast_dostatement, \
            xjs_ast_dostatement_ctor, \
            xjs_ast_dostatement_dtor);
    return r;
}

/* AST : For Statement */

static void xjs_ast_forstatement_ctor(void *data)
{
    xjs_ast_forstatement_ref r = data;
    r->init.type = xjs_ast_forstatement_init_null;
    r->test.type = xjs_ast_forstatement_test_null;
    r->update.type = xjs_ast_forstatement_update_null;
    r->body = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_forstatement_dtor(void *data)
{
    xjs_ast_forstatement_ref r = data;
    switch (r->init.type)
    {
        case xjs_ast_forstatement_init_expr:
            ec_delete(r->init.u.as_expr);
            break;
        case xjs_ast_forstatement_init_vardecl:
            ec_delete(r->init.u.as_vardecl);
            break;
        case xjs_ast_forstatement_init_null:
            break;
    }
    switch (r->test.type)
    {
        case xjs_ast_forstatement_test_expr:
            ec_delete(r->test.u.as_expr);
        case xjs_ast_forstatement_test_null:
            break;
    }
    switch (r->update.type)
    {
        case xjs_ast_forstatement_update_expr:
            ec_delete(r->update.u.as_expr);
        case xjs_ast_forstatement_update_null:
            break;
    }
    ec_delete(r->body);
}

xjs_ast_forstatement_ref xjs_ast_forstatement_new(void)
{
    xjs_ast_forstatement_ref r = ec_newcd(xjs_ast_forstatement, \
            xjs_ast_forstatement_ctor, \
            xjs_ast_forstatement_dtor);
    return r;
}

/* AST : Statement */

static void xjs_ast_statement_expressionstatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_expressionstatement;
    r->u.xjs_ast_statement_expressionstatement = xjs_ast_expressionstatement_new();
}

static void xjs_ast_statement_expressionstatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_expressionstatement);
}

xjs_ast_statement_ref xjs_ast_statement_expressionstatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_expressionstatement_ctor, \
            xjs_ast_statement_expressionstatement_dtor);
    return r;
}

void xjs_ast_statement_expressionstatement_expression_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref expr)
{
    r->u.xjs_ast_statement_expressionstatement->expression = expr;
}

static void xjs_ast_statement_emptystatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_emptystatement;
    r->u.xjs_ast_statement_emptystatement = xjs_ast_emptystatement_new();
}

static void xjs_ast_statement_emptystatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_emptystatement);
}

xjs_ast_statement_ref xjs_ast_statement_emptystatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_emptystatement_ctor, \
            xjs_ast_statement_emptystatement_dtor);
    return r;
}

static void xjs_ast_statement_blockstatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_blockstatement;
    r->u.xjs_ast_statement_blockstatement = xjs_ast_blockstatement_new();
}

static void xjs_ast_statement_blockstatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_blockstatement);
}

xjs_ast_statement_ref xjs_ast_statement_blockstatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_blockstatement_ctor, \
            xjs_ast_statement_blockstatement_dtor);
    return r;
}

void xjs_ast_statement_blockstatement_push_back(xjs_ast_statement_ref r,
        xjs_ast_statementlistitem_ref new_item)
{
    xjs_ast_statementlist_push_back( \
            r->u.xjs_ast_statement_blockstatement->body, \
            new_item);
}

static void xjs_ast_statement_inspectstatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_inspectstatement;
    r->u.xjs_ast_statement_inspectstatement = xjs_ast_inspectstatement_new();
}

static void xjs_ast_statement_inspectstatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_inspectstatement);
}

xjs_ast_statement_ref xjs_ast_statement_inspectstatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_inspectstatement_ctor, \
            xjs_ast_statement_inspectstatement_dtor);
    return r;
}

void xjs_ast_statement_inspectstatement_argument_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref argument)
{
    r->u.xjs_ast_statement_inspectstatement->argument = argument;
}

static void xjs_ast_statement_returnstatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_returnstatement;
    r->u.xjs_ast_statement_returnstatement = xjs_ast_returnstatement_new();
}

static void xjs_ast_statement_returnstatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_returnstatement);
}

xjs_ast_statement_ref xjs_ast_statement_returnstatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_returnstatement_ctor, \
            xjs_ast_statement_returnstatement_dtor);
    return r;
}

void xjs_ast_statement_returnstatement_argument_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref argument)
{
    r->u.xjs_ast_statement_returnstatement->argument = argument;
}

static void xjs_ast_statement_breakstatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_breakstatement;
    r->u.xjs_ast_statement_breakstatement = xjs_ast_breakstatement_new();
}

static void xjs_ast_statement_breakstatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_breakstatement);
}

xjs_ast_statement_ref xjs_ast_statement_breakstatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_breakstatement_ctor, \
            xjs_ast_statement_breakstatement_dtor);
    return r;
}

static void xjs_ast_statement_ifstatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_ifstatement;
    r->u.xjs_ast_statement_ifstatement = xjs_ast_ifstatement_new();
}

static void xjs_ast_statement_continuestatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_continuestatement;
    r->u.xjs_ast_statement_continuestatement = xjs_ast_continuestatement_new();
}

static void xjs_ast_statement_continuestatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_continuestatement);
}

xjs_ast_statement_ref xjs_ast_statement_continuestatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_continuestatement_ctor, \
            xjs_ast_statement_continuestatement_dtor);
    return r;
}

static void xjs_ast_statement_ifstatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_ifstatement);
}

xjs_ast_statement_ref xjs_ast_statement_ifstatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_ifstatement_ctor, \
            xjs_ast_statement_ifstatement_dtor);
    return r;
}

void xjs_ast_statement_ifstatement_test_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref test)
{
    r->u.xjs_ast_statement_ifstatement->test = test;
}

void xjs_ast_statement_ifstatement_consequent_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref consequent)
{
    r->u.xjs_ast_statement_ifstatement->consequent = consequent;
}

void xjs_ast_statement_ifstatement_alternate_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref alternate)
{
    r->u.xjs_ast_statement_ifstatement->alternate = alternate;
}

static void xjs_ast_statement_whilestatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_whilestatement;
    r->u.xjs_ast_statement_whilestatement = xjs_ast_whilestatement_new();
}

static void xjs_ast_statement_whilestatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_whilestatement);
}

xjs_ast_statement_ref xjs_ast_statement_whilestatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_whilestatement_ctor, \
            xjs_ast_statement_whilestatement_dtor);
    return r;
}

void xjs_ast_statement_whilestatement_test_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref test)
{
    r->u.xjs_ast_statement_whilestatement->test = test;
}

void xjs_ast_statement_whilestatement_body_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref body)
{
    r->u.xjs_ast_statement_whilestatement->body = body;
}

static void xjs_ast_statement_dostatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_dostatement;
    r->u.xjs_ast_statement_dostatement = xjs_ast_dostatement_new();
}

static void xjs_ast_statement_dostatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_dostatement);
}

xjs_ast_statement_ref xjs_ast_statement_dostatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_dostatement_ctor, \
            xjs_ast_statement_dostatement_dtor);
    return r;
}

void xjs_ast_statement_dostatement_body_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref body)
{
    r->u.xjs_ast_statement_dostatement->body = body;
}

void xjs_ast_statement_dostatement_test_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref test)
{
    r->u.xjs_ast_statement_dostatement->test = test;
}

static void xjs_ast_statement_forstatement_ctor(void *data)
{
    xjs_ast_statement_ref r = data;
    r->tag = xjs_ast_statement_forstatement;
    r->u.xjs_ast_statement_forstatement = xjs_ast_forstatement_new();
}

static void xjs_ast_statement_forstatement_dtor(void *data)
{
    xjs_ast_statement_ref r = data;
    ec_delete(r->u.xjs_ast_statement_forstatement);
}

xjs_ast_statement_ref xjs_ast_statement_forstatement_new(void)
{
    xjs_ast_statement_ref r = ec_newcd(xjs_ast_statement, \
            xjs_ast_statement_forstatement_ctor, \
            xjs_ast_statement_forstatement_dtor);
    return r;
}

void xjs_ast_statement_forstatement_init_set_null( \
        xjs_ast_statement_ref r)
{
    r->u.xjs_ast_statement_forstatement->init.type = xjs_ast_forstatement_init_null;
}

void xjs_ast_statement_forstatement_init_set_expr( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref init)
{
    r->u.xjs_ast_statement_forstatement->init.type = xjs_ast_forstatement_init_expr;
    r->u.xjs_ast_statement_forstatement->init.u.as_expr = init;
}

void xjs_ast_statement_forstatement_init_set_vardecl( \
        xjs_ast_statement_ref r, \
        xjs_ast_variabledeclaration_ref vardecl)
{
    r->u.xjs_ast_statement_forstatement->init.type = xjs_ast_forstatement_init_vardecl;
    r->u.xjs_ast_statement_forstatement->init.u.as_vardecl = vardecl;
}

void xjs_ast_statement_forstatement_test_set_null( \
        xjs_ast_statement_ref r)
{
    r->u.xjs_ast_statement_forstatement->test.type = xjs_ast_forstatement_test_null;
}

void xjs_ast_statement_forstatement_test_set_expr( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref test)
{
    r->u.xjs_ast_statement_forstatement->test.type = xjs_ast_forstatement_test_expr;
    r->u.xjs_ast_statement_forstatement->test.u.as_expr = test;
}

void xjs_ast_statement_forstatement_update_set_null( \
        xjs_ast_statement_ref r)
{
    r->u.xjs_ast_statement_forstatement->update.type = xjs_ast_forstatement_update_null;
}

void xjs_ast_statement_forstatement_update_set_expr( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref update)
{
    r->u.xjs_ast_statement_forstatement->update.type = xjs_ast_forstatement_update_expr;
    r->u.xjs_ast_statement_forstatement->update.u.as_expr = update;
}

void xjs_ast_statement_forstatement_body_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref body)
{
    r->u.xjs_ast_statement_forstatement->body = body;
}

/* AST : Declaration */

static void xjs_ast_declaration_variabledeclaration_ctor(void *data)
{
    xjs_ast_declaration_ref r = data;
    r->tag = xjs_ast_declaration_variabledeclaration;
    r->u.xjs_ast_declaration_variabledeclaration = xjs_ast_variabledeclaration_new();
}

static void xjs_ast_declaration_variabledeclaration_dtor(void *data)
{
    xjs_ast_declaration_ref r = data;
    ec_delete(r->u.xjs_ast_declaration_variabledeclaration);
}

xjs_ast_declaration_ref xjs_ast_declaration_variabledeclaration_new(void)
{
    xjs_ast_declaration_ref r = ec_newcd(xjs_ast_declaration, \
            xjs_ast_declaration_variabledeclaration_ctor, \
            xjs_ast_declaration_variabledeclaration_dtor);
    return r;
}

/* AST : StatementListItem */

static void xjs_ast_statementlistitem_declaration_ctor(void *data)
{
    xjs_ast_statementlistitem_ref r = data;
    r->tag = xjs_ast_statementlistitem_declaration;
    r->u.xjs_ast_statementlistitem_declaration = NULL;
}

static void xjs_ast_statementlistitem_declaration_dtor(void *data)
{
    xjs_ast_statementlistitem_ref r = data;
    ec_delete(r->u.xjs_ast_statementlistitem_declaration);
}

xjs_ast_statementlistitem_ref xjs_ast_statementlistitem_declaration_new(void)
{
    xjs_ast_statementlistitem_ref r = ec_newcd(xjs_ast_statementlistitem, \
            xjs_ast_statementlistitem_declaration_ctor, \
            xjs_ast_statementlistitem_declaration_dtor);
    return r;
}

void xjs_ast_statementlistitem_declaration_declaration_set( \
        xjs_ast_statementlistitem_ref r, xjs_ast_declaration_ref decl)
{
    r->u.xjs_ast_statementlistitem_declaration = decl;
}

static void xjs_ast_statementlistitem_statement_ctor(void *data)
{
    xjs_ast_statementlistitem_ref r = data;
    r->tag = xjs_ast_statementlistitem_statement;
    r->u.xjs_ast_statementlistitem_statement = NULL;
}

static void xjs_ast_statementlistitem_statement_dtor(void *data)
{
    xjs_ast_statementlistitem_ref r = data;
    ec_delete(r->u.xjs_ast_statementlistitem_statement);
}

xjs_ast_statementlistitem_ref xjs_ast_statementlistitem_statement_new(void)
{
    xjs_ast_statementlistitem_ref r = ec_newcd(xjs_ast_statementlistitem, \
            xjs_ast_statementlistitem_statement_ctor, \
            xjs_ast_statementlistitem_statement_dtor);
    return r;
}

void xjs_ast_statementlistitem_statement_statement_set( \
        xjs_ast_statementlistitem_ref r, xjs_ast_statement_ref stmt)
{
    r->u.xjs_ast_statementlistitem_statement = stmt;
}

/* AST : StatementList */

static void xjs_ast_statementlist_node_dtor(xjs_ast_statementlistitem_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ast_statementlist, xjs_ast_statementlistitem_ref, xjs_ast_statementlist_node_dtor);

/* AST : ImportSpecifier */

static void xjs_ast_importspecifier_ctor(void *data)
{
    xjs_ast_importspecifier_ref r = data;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_importspecifier_dtor(void *data)
{
    xjs_ast_importspecifier_ref r = data;
    switch (r->type)
    {
        case xjs_ast_importspecifier_type_importspecifier:
            ec_delete(r->u.importspecifier.local);
            ec_delete(r->u.importspecifier.imported);
            break;
        case xjs_ast_importspecifier_type_importdefaultspecifier:
            ec_delete(r->u.importdefaultspecifier.local);
            break;
        case xjs_ast_importspecifier_type_importnamespacespecifier:
            break;
    }
}

xjs_ast_importspecifier_ref xjs_ast_importspecifier_importspecifier_new(void)
{
    xjs_ast_importspecifier_ref r = ec_newcd(xjs_ast_importspecifier, \
            xjs_ast_importspecifier_ctor, xjs_ast_importspecifier_dtor);
    r->type = xjs_ast_importspecifier_type_importspecifier;
    r->u.importspecifier.imported = NULL;
    r->u.importspecifier.local = NULL;
    return r;
}

xjs_ast_importspecifier_ref xjs_ast_importspecifier_importdefaultspecifier_new(void)
{
    xjs_ast_importspecifier_ref r = ec_newcd(xjs_ast_importspecifier, \
            xjs_ast_importspecifier_ctor, xjs_ast_importspecifier_dtor);
    r->type = xjs_ast_importspecifier_type_importdefaultspecifier;
    r->u.importdefaultspecifier.local = NULL;
    return r;
}

xjs_ast_importspecifier_ref xjs_ast_importspecifier_importnamespacespecifier_new(void)
{
    xjs_ast_importspecifier_ref r = ec_newcd(xjs_ast_importspecifier, \
            xjs_ast_importspecifier_ctor, xjs_ast_importspecifier_dtor);
    r->type = xjs_ast_importspecifier_type_importnamespacespecifier;
    return r;
}

static void xjs_ast_importspecifier_node_dtor(xjs_ast_importspecifier_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ast_importspecifierlist, xjs_ast_importspecifier_ref, xjs_ast_importspecifier_node_dtor);

/* AST : ImportDeclaration */

static void xjs_ast_importdeclaration_ctor(void *data)
{
    xjs_ast_importdeclaration_ref r = data;
    r->source = NULL;
    r->specifiers = xjs_ast_importspecifierlist_new();
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_importdeclaration_dtor(void *data)
{
    xjs_ast_importdeclaration_ref r = data;
    ec_delete(r->source);
    ec_delete(r->specifiers);
}

xjs_ast_importdeclaration_ref xjs_ast_importdeclaration_new(void)
{
    xjs_ast_importdeclaration_ref r = ec_newcd(xjs_ast_importdeclaration, \
            xjs_ast_importdeclaration_ctor, xjs_ast_importdeclaration_dtor);
    return r;
}

void xjs_ast_importdeclaration_source_set(xjs_ast_importdeclaration_ref r, \
        xjs_ast_literal_ref source)
{
    r->source = source;
}

void xjs_ast_importdeclaration_specifiers_push_back(xjs_ast_importdeclaration_ref r, \
        xjs_ast_importspecifier_ref new_importspecifier)
{
    ect_list_push_back(xjs_ast_importspecifierlist, r->specifiers, new_importspecifier);
}

/* AST : ExportSpecifier */

static void xjs_ast_exportspecifier_ctor(void *data)
{
    xjs_ast_exportspecifier_ref r = data;
    r->type = xjs_ast_exportspecifier_type_exportspecifier;
    r->exported = NULL;
    r->local = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_exportspecifier_dtor(void *data)
{
    xjs_ast_exportspecifier_ref r = data;
    ec_delete(r->exported);
    ec_delete(r->local);
}

xjs_ast_exportspecifier_ref xjs_ast_exportspecifier_new(void)
{
    xjs_ast_exportspecifier_ref r = ec_newcd(xjs_ast_exportspecifier, \
            xjs_ast_exportspecifier_ctor, xjs_ast_exportspecifier_dtor);
    return r;
}

static void xjs_ast_exportspecifier_node_dtor(xjs_ast_exportspecifier_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ast_exportspecifierlist, xjs_ast_exportspecifier_ref, xjs_ast_exportspecifier_node_dtor);

/* AST : ExportDeclaration */

static void xjs_ast_exportdefaultdeclaration_identifier_ctor(void *data)
{
    xjs_ast_exportdefaultdeclaration_ref r = data;
    r->type = xjs_opaque_ast_exportdefaultdeclaration_type_identifier;
    r->declaration.as_identifier = NULL;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_exportdefaultdeclaration_dtor(void *data)
{
    xjs_ast_exportdefaultdeclaration_ref r = data;
    switch (r->type)
    {
        case xjs_opaque_ast_exportdefaultdeclaration_type_identifier:
            ec_delete(r->declaration.as_identifier);
            break;
        case xjs_opaque_ast_exportdefaultdeclaration_type_expression:
            ec_delete(r->declaration.as_expression);
            break;
    }
}

xjs_ast_exportdefaultdeclaration_ref xjs_ast_exportdefaultdeclaration_identifier_new(void)
{
    xjs_ast_exportdefaultdeclaration_ref r = ec_newcd( \
            xjs_ast_exportdefaultdeclaration, \
            xjs_ast_exportdefaultdeclaration_identifier_ctor, \
            xjs_ast_exportdefaultdeclaration_dtor);
    r->type = xjs_opaque_ast_exportdefaultdeclaration_type_identifier;
    return r;
}

xjs_ast_exportdefaultdeclaration_ref xjs_ast_exportdefaultdeclaration_expression_new(void)
{
    xjs_ast_exportdefaultdeclaration_ref r = ec_newcd( \
            xjs_ast_exportdefaultdeclaration, \
            xjs_ast_exportdefaultdeclaration_identifier_ctor, \
            xjs_ast_exportdefaultdeclaration_dtor);
    r->type = xjs_opaque_ast_exportdefaultdeclaration_type_expression;
    return r;
}

static void xjs_ast_exportnameddeclaration_ctor(void *data)
{
    xjs_ast_exportnameddeclaration_ref r = data;
    r->source = NULL;
    r->specifiers = xjs_ast_exportspecifierlist_new();
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_exportnameddeclaration_dtor(void *data)
{
    xjs_ast_exportnameddeclaration_ref r = data;
    ec_delete(r->source);
    ec_delete(r->specifiers);
}

xjs_ast_exportnameddeclaration_ref xjs_ast_exportnameddeclaration_new(void)
{
    xjs_ast_exportnameddeclaration_ref r = ec_newcd( \
            xjs_ast_exportnameddeclaration, \
            xjs_ast_exportnameddeclaration_ctor, \
            xjs_ast_exportnameddeclaration_dtor);
    return r;
}

static void xjs_ast_exportdeclaration_ctor(void *data)
{
    xjs_ast_exportdeclaration_ref r = data;
    r->type = xjs_ast_exportdeclaration_type_unknown;
    xjs_ast_loc_init(&r->loc);
    xjs_ast_range_init(&r->range); 
}

static void xjs_ast_exportdeclaration_dtor(void *data)
{
    xjs_ast_exportdeclaration_ref r = data;
    switch (r->type)
    {
        case xjs_ast_exportdeclaration_type_unknown:
            break;
        case xjs_ast_exportdeclaration_type_all:
            break;
        case xjs_ast_exportdeclaration_type_default:
            ec_delete(r->u.as_default);
            break;
        case xjs_ast_exportdeclaration_type_named:
            ec_delete(r->u.as_named);
            break;
    }
}

xjs_ast_exportdeclaration_ref xjs_ast_exportdeclaration_default_new(void)
{
    xjs_ast_exportdeclaration_ref r = ec_newcd(xjs_ast_exportdeclaration, \
            xjs_ast_exportdeclaration_ctor, xjs_ast_exportdeclaration_dtor);
    r->type = xjs_ast_exportdeclaration_type_default;
    r->u.as_default = NULL;
    return r;
}

xjs_ast_exportdeclaration_ref xjs_ast_exportdeclaration_named_new(void)
{
    xjs_ast_exportdeclaration_ref r = ec_newcd(xjs_ast_exportdeclaration, \
            xjs_ast_exportdeclaration_ctor, xjs_ast_exportdeclaration_dtor);
    r->type = xjs_ast_exportdeclaration_type_named;
    r->u.as_named = NULL;
    return r;
}

void xjs_ast_exportdeclaration_default_set( \
        xjs_ast_exportdeclaration_ref r, \
        xjs_ast_exportdefaultdeclaration_ref defaultdecl)
{
    r->u.as_default = defaultdecl;
}

void xjs_ast_exportdeclaration_named_specifiers_push_back(xjs_ast_exportdeclaration_ref r, \
        xjs_ast_exportspecifier_ref new_exportspecifier)
{
    xjs_ast_exportspecifierlist_push_back(r->u.as_named->specifiers, new_exportspecifier);
}

/* AST : ModuleItem */

static void xjs_ast_moduleitem_ctor(void *data)
{
    xjs_ast_moduleitem_ref r = data;
    (void)r;
}

static void xjs_ast_moduleitem_dtor(void *data)
{
    xjs_ast_moduleitem_ref r = data;
    switch (r->tag)
    {
        case xjs_ast_moduleitem_importdeclaration:
            ec_delete(r->u.xjs_ast_moduleitem_importdeclaration);
            break;
        case xjs_ast_moduleitem_exportdeclaration:
            ec_delete(r->u.xjs_ast_moduleitem_exportdeclaration);
            break;
        case xjs_ast_moduleitem_statementlistitem:
            ec_delete(r->u.xjs_ast_moduleitem_statementlistitem);
            break;
    }
}

xjs_ast_moduleitem_ref xjs_ast_moduleitem_importdeclaration_new(void)
{
    xjs_ast_moduleitem_ref r = ec_newcd(xjs_ast_moduleitem, \
            xjs_ast_moduleitem_ctor, \
            xjs_ast_moduleitem_dtor);
    r->tag = xjs_ast_moduleitem_importdeclaration;
    r->u.xjs_ast_moduleitem_importdeclaration = NULL;
    return r;
}

void xjs_ast_moduleitem_importdeclaration_set( \
        xjs_ast_moduleitem_ref r, \
        xjs_ast_importdeclaration_ref importdeclaration)
{
    r->u.xjs_ast_moduleitem_importdeclaration = importdeclaration;
}

xjs_ast_moduleitem_ref xjs_ast_moduleitem_exportdeclaration_new(void)
{
    xjs_ast_moduleitem_ref r = ec_newcd(xjs_ast_moduleitem, \
            xjs_ast_moduleitem_ctor, \
            xjs_ast_moduleitem_dtor);
    r->tag = xjs_ast_moduleitem_exportdeclaration;
    return r;
}

void xjs_ast_moduleitem_exportdeclaration_set( \
        xjs_ast_moduleitem_ref r, \
        xjs_ast_exportdeclaration_ref exportdeclaration)
{
    r->u.xjs_ast_moduleitem_exportdeclaration = exportdeclaration;
}

xjs_ast_moduleitem_ref xjs_ast_moduleitem_statementlistitem_new(void)
{
    xjs_ast_moduleitem_ref r = ec_newcd(xjs_ast_moduleitem, \
            xjs_ast_moduleitem_ctor, \
            xjs_ast_moduleitem_dtor);
    r->tag = xjs_ast_moduleitem_statementlistitem;
    r->u.xjs_ast_moduleitem_statementlistitem = NULL;
    return r;
}

void xjs_ast_moduleitem_statementlistitem_set( \
        xjs_ast_moduleitem_ref r, \
        xjs_ast_statementlistitem_ref new_item)
{
    r->u.xjs_ast_moduleitem_statementlistitem = new_item;
}

/* AST : ImportSpecifierList */

static void xjs_ast_moduleitemlist_node_dtor(xjs_ast_moduleitem_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_ast_moduleitemlist, xjs_ast_moduleitem_ref, xjs_ast_moduleitemlist_node_dtor);

/* AST : Program */

static void xjs_ast_program_script_ctor(void *data)
{
    xjs_ast_program_ref r = data;
    r->tag = xjs_ast_program_script;
    r->u.xjs_ast_program_script.body = xjs_ast_statementlist_new();
    xjs_ast_loc_init(&r->u.xjs_ast_program_script.loc);
    xjs_ast_range_init(&r->u.xjs_ast_program_script.range); 
}

static void xjs_ast_program_script_dtor(void *data)
{
    xjs_ast_program_ref r = data;
    ec_delete(r->u.xjs_ast_program_script.body);
}

xjs_ast_program_ref xjs_ast_program_script_new(void)
{
    xjs_ast_program_ref r = ec_newcd(xjs_ast_program, \
            xjs_ast_program_script_ctor, xjs_ast_program_script_dtor);
    return r;
}

void xjs_ast_program_script_push_back( \
        xjs_ast_program_ref program, \
        xjs_ast_statementlistitem_ref new_item)
{
    xjs_ast_statementlist_push_back( \
            program->u.xjs_ast_program_script.body, 
            new_item);
}

/* AST : Module */

static void xjs_ast_program_module_ctor(void *data)
{
    xjs_ast_program_ref r = data;
    r->tag = xjs_ast_program_module;
    r->u.xjs_ast_program_module.body = xjs_ast_moduleitemlist_new();
    xjs_ast_loc_init(&r->u.xjs_ast_program_module.loc);
    xjs_ast_range_init(&r->u.xjs_ast_program_module.range);
}

static void xjs_ast_program_module_dtor(void *data)
{
    xjs_ast_program_ref r = data;
    ec_delete(r->u.xjs_ast_program_module.body);
}

xjs_ast_program_ref xjs_ast_program_module_new(void)
{
    xjs_ast_program_ref r = ec_newcd(xjs_ast_program, \
            xjs_ast_program_module_ctor, xjs_ast_program_module_dtor);
    return r;
}

void xjs_ast_program_module_push_back( \
        xjs_ast_program_ref program, \
        xjs_ast_moduleitem_ref new_item)
{
    xjs_ast_moduleitemlist_push_back( \
            program->u.xjs_ast_program_module.body, 
            new_item);
}

