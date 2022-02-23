/* XenonJS : AST Printer
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_encoding.h>
#include <ec_list.h>
#include <ec_map.h>
#include <ec_string.h>
#include "xjs.h"
#include "xjs_ir.h"
#include "xjs_helper.h"
#include "xjs_aux.h"
#include "xjs_astprinter.h"

static int xjs_ast_serialize_statementlistitem( \
        ec_string *result, xjs_ast_statementlistitem_ref item);
static int xjs_ast_serialize_expression( \
        ec_string *result, xjs_ast_expression_ref expr);


static int xjs_ast_serialize_append_unescaped_json_string( \
        ec_string *result, ec_string *value_string)
{
    ect_iterator(ec_string) it;
    ec_char_t ch;

    ect_for(ec_string, value_string, it)
    {
        ch = ect_deref(ec_char_t , it);
        if (ch == '"')
        {
            ec_string_push_back(result, '\\');
            ec_string_push_back(result, '\"');
        }
        else if (ch == '\\')
        {
            ec_string_push_back(result, '\\');
            ec_string_push_back(result, '\\');
        }
        else
        {
            ec_string_push_back(result, ch);
        }
    }

    return 0;
}

static int xjs_ast_serialize_append_number( \
        ec_string *result, double value)
{
    return xjs_aux_serialize_append_number(result, value);
}

static int xjs_ast_serialize_append_int( \
        ec_string *result, int value)
{
    return xjs_aux_serialize_append_int(result, value);
}

static int xjs_ast_serialize_loc( \
        ec_string *result, xjs_ast_loc_ref loc)
{
    ec_string_append_c_str(result, "\"loc\":{\"start\":{\"line\":");
    xjs_ast_serialize_append_int(result, \
            loc->start.ln);
    ec_string_append_c_str(result, ",\"column\":");
    xjs_ast_serialize_append_int(result, \
            loc->start.col);
    ec_string_append_c_str(result, "}");
    ec_string_append_c_str(result, ",\"end\":{\"line\":");
    xjs_ast_serialize_append_int(result, \
            loc->end.ln);
    ec_string_append_c_str(result, ",\"column\":");
    xjs_ast_serialize_append_int(result, \
            loc->end.col);
    ec_string_append_c_str(result, "}}");

    return 0;
}

static int xjs_ast_serialize_variabledeclaration( \
        ec_string *result, xjs_ast_variabledeclaration_ref decl)
{
    int ret = 0;
    ect_iterator(xjs_ast_variabledeclaratorlist) it;
    xjs_bool first = xjs_true;

    ec_string_append_c_str(result, "{\"type\":\"VariableDeclaration\",\"declarations\":[");

    ect_for(xjs_ast_variabledeclaratorlist, decl->declarations, it)
    {
        xjs_ast_variabledeclarator_ref declarator;

        if (first) first = xjs_false; else { ec_string_push_back(result, ','); }

        ec_string_append_c_str(result, "{\"type\":\"VariableDeclarator\",\"id\":");
        declarator = ect_deref(xjs_ast_variabledeclarator_ref, it);
        switch (declarator->id->tag)
        {
            case xjs_ast_variabledeclarator_id_identifier:
                ec_string_append_c_str(result, "{\"type\":\"Identifier\",\"name\":\"");
                ec_string_append(result, declarator->id->u.xjs_ast_variabledecorator_identifier->name);
                ec_string_append_c_str(result, "\"");
                {
                    ec_string_append_c_str(result, ",");
                    xjs_ast_serialize_loc(result, &declarator->id->u.xjs_ast_variabledecorator_identifier->loc);
                }
                ec_string_append_c_str(result, "}");
                break;

            case xjs_ast_variabledeclarator_id_bindingpattern:
                ret = -1;
                goto fail;
        }
        ec_string_append_c_str(result, ",\"init\":");
        if (declarator->init == NULL)
        {
            ec_string_append_c_str(result, "null");
        }
        else
        {
            if (xjs_ast_serialize_expression(result, declarator->init) != 0)
            { ret = -1; goto fail; }
        }
        {
            ec_string_append_c_str(result, ",");
            xjs_ast_serialize_loc(result, &declarator->loc);
        }
        ec_string_append_c_str(result, "}");
    }
    ec_string_push_back(result, ']');

    switch (decl->kind)
    {
        case xjs_ast_variabledeclaration_kind_var:
            ec_string_append_c_str(result, ",\"kind\":\"var\"");
            break;
        case xjs_ast_variabledeclaration_kind_const:
            ec_string_append_c_str(result, ",\"kind\":\"const\"");
            break;
        case xjs_ast_variabledeclaration_kind_let:
            ec_string_append_c_str(result, ",\"kind\":\"let\"");
            break;
    }
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &decl->loc);
    }
    ec_string_push_back(result, '}');

fail:
    return ret;
}

static int xjs_ast_serialize_declaration( \
        ec_string *result, xjs_ast_declaration_ref decl)
{
    int ret = 0;

    switch (decl->tag)
    {
        case xjs_ast_declaration_classdeclaration:
            ret = -1;
            break;

        case xjs_ast_declaration_functiondeclaration:
            ret = -1;
            break;

        case xjs_ast_declaration_variabledeclaration:
            ret = xjs_ast_serialize_variabledeclaration( \
                    result, decl->u.xjs_ast_declaration_variabledeclaration);
            break;
    }

    return ret;
}

static int xjs_ast_serialize_property( \
        ec_string *result, xjs_ast_property_ref prop)
{
    ec_string_append_c_str(result, "{\"type\":\"Property\",\"key\":");
    xjs_ast_serialize_expression(result, prop->key);
    ec_string_append_c_str(result, ",\"value\":");
    xjs_ast_serialize_expression(result, prop->value);
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_identifier( \
        ec_string *result, xjs_ast_identifier_ref identifier)
{
    ec_string_append_c_str(result, "{\"type\":\"Identifier\",\"name\":\"");
    ec_string_append(result, identifier->name);
    ec_string_append_c_str(result, "\"");
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &identifier->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_literal( \
        ec_string *result, xjs_ast_literal_ref literal)
{
    ec_string_append_c_str(result, "{\"type\":\"Literal\"");
    switch (literal->type)
    {
        case xjs_ast_literal_unknown:
            break;
        case xjs_ast_literal_undefined:
            break;
        case xjs_ast_literal_null:
            ec_string_append_c_str(result, ",\"value\":null");
            break;
        case xjs_ast_literal_boolean:
            if (literal->value.as_bool == ec_false)
            { ec_string_append_c_str(result, ",\"value\":false"); }
            else
            { ec_string_append_c_str(result, ",\"value\":true"); }
            break;
        case xjs_ast_literal_number:
            ec_string_append_c_str(result, ",\"value\":");
            xjs_ast_serialize_append_number(result, literal->value.as_number);
            break;
        case xjs_ast_literal_string:
            ec_string_append_c_str(result, ",\"value\":\"");
            xjs_ast_serialize_append_unescaped_json_string( \
                    result, literal->value.as_string);
            ec_string_append_c_str(result, "\"");
            break;
    }
    ec_string_append_c_str(result, ",\"raw\":\"");
    xjs_ast_serialize_append_unescaped_json_string(result, literal->raw);
    ec_string_append_c_str(result, "\"");
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &literal->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_expression( \
        ec_string *result, xjs_ast_expression_ref expr)
{
    int ret = 0;

    switch (expr->tag)
    {
        case xjs_ast_expression_literal:
            xjs_ast_serialize_literal(result, expr->u.xjs_ast_expression_literal);
            break;

        case xjs_ast_expression_identifier:
            ec_string_append_c_str(result, "{\"type\":\"Identifier\",\"name\":\"");
            ec_string_append(result, expr->u.xjs_ast_expression_identifier->name);
            ec_string_append_c_str(result, "\"");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_identifier->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_unaryexpression:
            ec_string_append_c_str(result, "{\"type\":\"UnaryExpression\",\"operator\":\"");
            ec_string_append(result, expr->u.xjs_ast_expression_unaryexpression->op);
            ec_string_append_c_str(result, "\",\"argument\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_unaryexpression->argument);
            ec_string_append_c_str(result, ",\"prefix\":");
            ec_string_append_c_str(result, "true");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_unaryexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_binaryexpression:
            ec_string_append_c_str(result, "{\"type\":\"BinaryExpression\",\"operator\":\"");
            ec_string_append(result, expr->u.xjs_ast_expression_binaryexpression->op);
            ec_string_append_c_str(result, "\",\"left\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_binaryexpression->left);
            ec_string_append_c_str(result, ",\"right\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_binaryexpression->right);
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_binaryexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_logicalexpression:
            ec_string_append_c_str(result, "{\"type\":\"LogicalExpression\",\"operator\":\"");
            ec_string_append(result, expr->u.xjs_ast_expression_logicalexpression->op);
            ec_string_append_c_str(result, "\",\"left\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_logicalexpression->left);
            ec_string_append_c_str(result, ",\"right\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_logicalexpression->right);
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_logicalexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_assignmentexpression:
            ec_string_append_c_str(result, "{\"type\":\"AssignmentExpression\",\"operator\":\"");
            ec_string_append(result, expr->u.xjs_ast_expression_assignmentexpression->op);
            ec_string_append_c_str(result, "\",\"left\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_assignmentexpression->left);
            ec_string_append_c_str(result, ",\"right\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_assignmentexpression->right);
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_assignmentexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_updateexpression:
            ec_string_append_c_str(result, "{\"type\":\"UpdateExpression\",\"operator\":\"");
            ec_string_append(result, expr->u.xjs_ast_expression_updateexpression->op);
            ec_string_append_c_str(result, "\",\"argument\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_updateexpression->argument);
            ec_string_append_c_str(result, ",\"prefix\":");
            ec_string_append_c_str(result, (expr->u.xjs_ast_expression_updateexpression->prefix == ec_false) ? "false" : "true");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_updateexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_functionexpression:
            ec_string_append_c_str(result, "{\"type\":\"FunctionExpression\",\"id\":");
            if (expr->u.xjs_ast_expression_functionexpression->id == NULL)
            {
                ec_string_append_c_str(result, "null");
            }
            else
            {
                ec_string_append_c_str(result, "\"");
                ec_string_append(result, expr->u.xjs_ast_expression_functionexpression->id);
                ec_string_append_c_str(result, "\"");
            }
            ec_string_append_c_str(result, ",\"params\":[");
            {
                ec_bool first2 = ec_true;
                ect_iterator(xjs_ast_parameterlist) it_parameter;
                ect_for(xjs_ast_parameterlist, \
                        expr->u.xjs_ast_expression_functionexpression->params, \
                        it_parameter)
                {
                    if (first2 == ec_true) first2 = ec_false; else ec_string_append_c_str(result, ",");
                    xjs_ast_parameter_ref parameter = ect_deref(xjs_ast_parameter_ref, it_parameter);
                    xjs_ast_serialize_identifier(result, parameter->id);
                }
            }
            ec_string_append_c_str(result, "]");
            ec_string_append_c_str(result, ",\"body\":");
            if (xjs_ast_serialize_statementlistitem(result, expr->u.xjs_ast_expression_functionexpression->body) != 0)
            { return -1; }
            ec_string_append_c_str(result, ",\"generator\":false");
            ec_string_append_c_str(result, ",\"expression\":false");
            ec_string_append_c_str(result, ",\"async\":false");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_functionexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_arrowfunctionexpression:
            ec_string_append_c_str(result, "{\"type\":\"ArrowFunctionExpression\",\"id\":null,\"params\":[");
            {
                ec_bool first2 = ec_true;
                ect_iterator(xjs_ast_parameterlist) it_parameter;
                ect_for(xjs_ast_parameterlist, \
                        expr->u.xjs_ast_expression_arrowfunctionexpression->params, \
                        it_parameter)
                {
                    if (first2 == ec_true) first2 = ec_false; else ec_string_append_c_str(result, ",");
                    xjs_ast_parameter_ref parameter = ect_deref(xjs_ast_parameter_ref, it_parameter);
                    ec_string_append_c_str(result, "{\"type\":\"Identifier\",\"name\":\"");
                    ec_string_append(result, parameter->id->name);
                    ec_string_append_c_str(result, "\"}");
                }
            }
            ec_string_append_c_str(result, "],\"body\":");
            switch (expr->u.xjs_ast_expression_arrowfunctionexpression->body.type)
            {
                case xjs_opaque_ast_arrowfunctionexpression_body_blockstmt:
                    if (xjs_ast_serialize_statementlistitem(result, \
                                expr->u.xjs_ast_expression_arrowfunctionexpression->body.u.as_blockstmt) != 0)
                    { return -1; }
                    break;
                case xjs_opaque_ast_arrowfunctionexpression_body_expr:
                    if (xjs_ast_serialize_expression(result, \
                                expr->u.xjs_ast_expression_arrowfunctionexpression->body.u.as_expr) != 0)
                    { return -1; }
                    break;
            }
            ec_string_append_c_str(result, ",\"generator\":false");
            switch (expr->u.xjs_ast_expression_arrowfunctionexpression->body.type)
            {
                case xjs_opaque_ast_arrowfunctionexpression_body_blockstmt:
                    ec_string_append_c_str(result, ",\"expression\":false");
                    break;
                case xjs_opaque_ast_arrowfunctionexpression_body_expr:
                    ec_string_append_c_str(result, ",\"expression\":true");
                    break;
            }
            ec_string_append_c_str(result, ",\"async\":false");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_arrowfunctionexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_objectexpression:
            ec_string_append_c_str(result, "{\"type\":\"ObjectExpression\",\"properties\":[");
            {
                xjs_ast_propertylist_ref properties = expr->u.xjs_ast_expression_objectexpression->properties;
                xjs_bool first = xjs_true;
                ect_iterator(xjs_ast_propertylist) it;
                ect_for(xjs_ast_propertylist, properties, it)
                {
                    xjs_ast_property_ref prop = NULL;
                    prop = ect_deref(xjs_ast_property_ref, it);

                    if (first) first = xjs_false; else { ec_string_push_back(result, ','); }

                    if (xjs_ast_serialize_property(result, prop) != 0)
                    { return -1; }
                }
            }
            ec_string_append_c_str(result, "]");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_objectexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_arrayexpression:
            ec_string_append_c_str(result, "{\"type\":\"ArrayExpression\",\"elements\":[");
            {
                xjs_ast_expressionlist_ref elements = expr->u.xjs_ast_expression_arrayexpression->elements;
                xjs_bool first = xjs_true;
                ect_iterator(xjs_ast_expressionlist) it;
                ect_for(xjs_ast_expressionlist, elements, it)
                {
                    xjs_ast_expression_ref element = NULL;
                    element = ect_deref(xjs_ast_expression_ref, it);

                    if (first) first = xjs_false; else { ec_string_push_back(result, ','); }

                    if (xjs_ast_serialize_expression(result, element) != 0)
                    { return -1; }
                }
            }
            ec_string_append_c_str(result, "]");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_arrayexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_callexpression:
            ec_string_append_c_str(result, "{\"type\":\"CallExpression\"");
            ec_string_append_c_str(result, ",\"callee\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_callexpression->callee);
            ec_string_append_c_str(result, ",\"arguments\":[");
            {
                xjs_bool first = xjs_true;
                xjs_ast_expression_ref expr_arg; 
                ect_iterator(xjs_ast_expressionlist) it;
                ect_for(xjs_ast_expressionlist, expr->u.xjs_ast_expression_callexpression->arguments, it)
                {
                    if (first) first = xjs_false; else { ec_string_push_back(result, ','); }
                     expr_arg = ect_deref(xjs_ast_expression_ref, it);
                    xjs_ast_serialize_expression(result, expr_arg);
                }
            }
            ec_string_append_c_str(result, "]");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_callexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_newexpression:
            ec_string_append_c_str(result, "{\"type\":\"NewExpression\"");
            ec_string_append_c_str(result, ",\"callee\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_newexpression->callee);
            ec_string_append_c_str(result, ",\"arguments\":[");
            {
                xjs_bool first = xjs_true;
                xjs_ast_expression_ref expr_arg; 
                ect_iterator(xjs_ast_expressionlist) it;
                ect_for(xjs_ast_expressionlist, expr->u.xjs_ast_expression_newexpression->arguments, it)
                {
                    if (first) first = xjs_false; else { ec_string_push_back(result, ','); }
                     expr_arg = ect_deref(xjs_ast_expression_ref, it);
                    xjs_ast_serialize_expression(result, expr_arg);
                }
            }
            ec_string_append_c_str(result, "]");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_newexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_memberexpression:
            ec_string_append_c_str(result, "{\"type\":\"MemberExpression\"");
            ec_string_append_c_str(result, ",\"computed\":");
            if (expr->u.xjs_ast_expression_memberexpression->computed == ec_false)
            { ec_string_append_c_str(result, "false"); }
            else
            { ec_string_append_c_str(result, "true"); }
            ec_string_append_c_str(result, ",\"object\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_memberexpression->object);
            ec_string_append_c_str(result, ",\"property\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_memberexpression->property);
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_memberexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_conditionalexpression:
            ec_string_append_c_str(result, "{\"type\":\"ConditionalExpression\"");
            ec_string_append_c_str(result, ",\"test\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_conditionalexpression->test);
            ec_string_append_c_str(result, ",\"consequent\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_conditionalexpression->consequent);
            ec_string_append_c_str(result, ",\"alternate\":");
            xjs_ast_serialize_expression(result, expr->u.xjs_ast_expression_conditionalexpression->alternate);
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_conditionalexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_expression_thisexpression:
            ec_string_append_c_str(result, "{\"type\":\"ThisExpression\"");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &expr->u.xjs_ast_expression_thisexpression->loc);
            }
            ec_string_append_c_str(result, "}");
            break;
    }

    return ret;
}

static int xjs_ast_serialize_emptystatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    ec_string_append_c_str(result, "{\"type\":\"EmptyStatement\"");
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &stmt->u.xjs_ast_statement_emptystatement->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_blockstatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_blockstatement_ref block_stmt = stmt->u.xjs_ast_statement_blockstatement;

    ec_string_append_c_str(result, "{\"type\":\"BlockStatement\",\"body\":[");
    {
        xjs_bool first = xjs_true;
        ect_iterator(xjs_ast_statementlist) it;
        ect_for(xjs_ast_statementlist, block_stmt->body, it)
        {
            xjs_ast_statementlistitem_ref item = ect_deref(xjs_ast_statementlistitem_ref, it);
            if (first) first = xjs_false; else { ec_string_push_back(result, ','); }
            xjs_ast_serialize_statementlistitem(result, item);
        }
    }
    ec_string_append_c_str(result, "]");
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &stmt->u.xjs_ast_statement_blockstatement->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_ifstatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_ifstatement_ref if_stmt = stmt->u.xjs_ast_statement_ifstatement;

    ec_string_append_c_str(result, "{\"type\":\"IfStatement\",\"test\":");
    xjs_ast_serialize_expression(result, if_stmt->test);
    ec_string_append_c_str(result, ",\"consequent\":");
    xjs_ast_serialize_statementlistitem(result, if_stmt->consequent);
    ec_string_append_c_str(result, ",\"alternate\":");
    if (if_stmt->alternate == NULL)
    {
        ec_string_append_c_str(result, "null");
    }
    else
    {
        xjs_ast_serialize_statementlistitem(result, if_stmt->alternate);
    }
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &if_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_whilestatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_whilestatement_ref while_stmt = stmt->u.xjs_ast_statement_whilestatement;

    ec_string_append_c_str(result, "{\"type\":\"WhileStatement\",\"test\":");
    xjs_ast_serialize_expression(result, while_stmt->test);
    ec_string_append_c_str(result, ",\"body\":");
    xjs_ast_serialize_statementlistitem(result, while_stmt->body);
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &while_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_dostatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_dostatement_ref do_stmt = stmt->u.xjs_ast_statement_dostatement;

    ec_string_append_c_str(result, "{\"type\":\"DoWhileStatement\",\"body\":");
    xjs_ast_serialize_statementlistitem(result, do_stmt->body);
    ec_string_append_c_str(result, ",\"test\":");
    xjs_ast_serialize_expression(result, do_stmt->test);
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &do_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_forstatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_forstatement_ref for_stmt = stmt->u.xjs_ast_statement_forstatement;

    ec_string_append_c_str(result, "{\"type\":\"ForStatement\",\"init\":");
    switch (for_stmt->init.type)
    {
        case xjs_ast_forstatement_init_expr:
            if (xjs_ast_serialize_expression(result, for_stmt->init.u.as_expr) != 0)
            { return -1; }
            break;
        case xjs_ast_forstatement_init_vardecl:
            if (xjs_ast_serialize_variabledeclaration(result, for_stmt->init.u.as_vardecl) != 0)
            { return -1; }
            break;
        case xjs_ast_forstatement_init_null:
            ec_string_append_c_str(result, "null");
            break;
    }
    ec_string_append_c_str(result, ",\"test\":");
    switch (for_stmt->test.type)
    {
        case xjs_ast_forstatement_test_expr:
            if (xjs_ast_serialize_expression(result, for_stmt->test.u.as_expr) != 0)
            { return -1; }
            break;
        case xjs_ast_forstatement_test_null:
            ec_string_append_c_str(result, "null");
            break;
    }
    ec_string_append_c_str(result, ",\"update\":");
    switch (for_stmt->update.type)
    {
        case xjs_ast_forstatement_update_expr:
            if (xjs_ast_serialize_expression(result, for_stmt->update.u.as_expr) != 0)
            { return -1; }
            break;
        case xjs_ast_forstatement_update_null:
            ec_string_append_c_str(result, "null");
            break;
    }

    ec_string_append_c_str(result, ",\"body\":");
    xjs_ast_serialize_statementlistitem(result, for_stmt->body);
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &for_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_inspectstatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_inspectstatement_ref inspect_stmt = stmt->u.xjs_ast_statement_inspectstatement;

    ec_string_append_c_str(result, "{\"type\":\"InspectStatement\",\"argument\":");
    xjs_ast_serialize_expression(result, inspect_stmt->argument);
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &inspect_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_returnstatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_returnstatement_ref return_stmt = stmt->u.xjs_ast_statement_returnstatement;

    ec_string_append_c_str(result, "{\"type\":\"ReturnStatement\",\"argument\":");
    xjs_ast_serialize_expression(result, return_stmt->argument);
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &return_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_breakstatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_breakstatement_ref break_stmt = stmt->u.xjs_ast_statement_breakstatement;

    ec_string_append_c_str(result, "{\"type\":\"BreakStatement\",\"label\":null");
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &break_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_continuestatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    xjs_ast_continuestatement_ref continue_stmt = stmt->u.xjs_ast_statement_continuestatement;

    ec_string_append_c_str(result, "{\"type\":\"ContinueStatement\",\"label\":null");
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &continue_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_expressionstatement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    int ret = 0;
    xjs_ast_expressionstatement_ref expr_stmt = stmt->u.xjs_ast_statement_expressionstatement;

    ec_string_append_c_str(result, "{\"type\":\"ExpressionStatement\",\"expression\":");
    ret = xjs_ast_serialize_expression(result, expr_stmt->expression);
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &expr_stmt->loc);
    }
    ec_string_append_c_str(result, "}");

    return ret;
}

static int xjs_ast_serialize_statement( \
        ec_string *result, xjs_ast_statement_ref stmt)
{
    int ret = 0;

        switch (stmt->tag)
        {
            case xjs_ast_statement_expressionstatement:
                ret = xjs_ast_serialize_expressionstatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_emptystatement:
                ret = xjs_ast_serialize_emptystatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_blockstatement:
                ret = xjs_ast_serialize_blockstatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_ifstatement:
                ret = xjs_ast_serialize_ifstatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_whilestatement:
                ret = xjs_ast_serialize_whilestatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_dostatement:
                ret = xjs_ast_serialize_dostatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_forstatement:
                ret = xjs_ast_serialize_forstatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_inspectstatement:
                ret = xjs_ast_serialize_inspectstatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_returnstatement:
                ret = xjs_ast_serialize_returnstatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_breakstatement:
                ret = xjs_ast_serialize_breakstatement( \
                        result, stmt);
                break;
            case xjs_ast_statement_continuestatement:
                ret = xjs_ast_serialize_continuestatement( \
                        result, stmt);
                break;
        }

    return ret;
}

static int xjs_ast_serialize_statementlistitem( \
        ec_string *result, xjs_ast_statementlistitem_ref item)
{
    int ret = 0;

    switch (item->tag)
    {
        case xjs_ast_statementlistitem_statement:
            ret = xjs_ast_serialize_statement( \
                    result, item->u.xjs_ast_statementlistitem_statement);
            break;

        case xjs_ast_statementlistitem_declaration:
            ret = xjs_ast_serialize_declaration( \
                    result, item->u.xjs_ast_statementlistitem_declaration);
            break;
    } 

    return ret;
}

static int xjs_ast_serialize_moduleitem_importdecl( \
        ec_string *result, xjs_ast_importdeclaration_ref import_decl)
{
    ec_string_append_c_str(result, "{\"type\":\"ImportDeclaration\",\"specifiers\":[");
    {
        xjs_bool first = xjs_true;
        ect_iterator(xjs_ast_importspecifierlist) it;
        ect_for(xjs_ast_importspecifierlist, import_decl->specifiers, it)
        {
            xjs_ast_importspecifier_ref specifier = ect_deref(xjs_ast_importspecifier_ref, it);
            if (first == xjs_true) first = xjs_false; else ec_string_append_c_str(result, ",");
            switch (specifier->type)
            {
                case xjs_ast_importspecifier_type_importspecifier:
                    ec_string_append_c_str(result, "{\"type\":\"ImportSpecifier\",\"local\":");
                    xjs_ast_serialize_identifier(result, specifier->u.importspecifier.local);
                    ec_string_append_c_str(result, ",\"imported\":");
                    xjs_ast_serialize_identifier(result, specifier->u.importspecifier.local);
                    {
                        ec_string_append_c_str(result, ",");
                        xjs_ast_serialize_loc(result, &specifier->loc);
                    }
                    ec_string_append_c_str(result, "}");
                    break;
                case xjs_ast_importspecifier_type_importdefaultspecifier:
                    ec_string_append_c_str(result, "{\"type\":\"ImportDefaultSpecifier\",\"local\":");
                    xjs_ast_serialize_identifier(result, specifier->u.importspecifier.local);
                    {
                        ec_string_append_c_str(result, ",");
                        xjs_ast_serialize_loc(result, &specifier->loc);
                    }
                    ec_string_append_c_str(result, "}");
                    break;
                case xjs_ast_importspecifier_type_importnamespacespecifier:
                    break;
            }
        }
    }
    ec_string_append_c_str(result, "],\"source\":");
    if (xjs_ast_serialize_literal(result, import_decl->source) != 0)
    { return -1; }
    {
        ec_string_append_c_str(result, ",");
        xjs_ast_serialize_loc(result, &import_decl->loc);
    }
    ec_string_append_c_str(result, "}");

    return 0;
}

static int xjs_ast_serialize_moduleitem_exportdecl( \
        ec_string *result, xjs_ast_exportdeclaration_ref export_decl)
{
    switch (export_decl->type)
    {
        case xjs_ast_exportdeclaration_type_unknown:
            break;
        case xjs_ast_exportdeclaration_type_all:
            break;
        case xjs_ast_exportdeclaration_type_default:
            ec_string_append_c_str(result, "{\"type\":\"ExportDefaultDeclaration\"");
            ec_string_append_c_str(result, ",\"declaration\":");
            switch (export_decl->u.as_default->type)
            {
                case xjs_opaque_ast_exportdefaultdeclaration_type_identifier:
                    xjs_ast_serialize_identifier(result, export_decl->u.as_default->declaration.as_identifier);
                    {
                        ec_string_append_c_str(result, ",");
                        xjs_ast_serialize_loc(result, &export_decl->loc);
                    }
                    break;
                case xjs_opaque_ast_exportdefaultdeclaration_type_expression:
                    xjs_ast_serialize_expression(result, export_decl->u.as_default->declaration.as_expression);
                    break;
            }
            ec_string_append_c_str(result, "}");
            break;
        case xjs_ast_exportdeclaration_type_named:
            ec_string_append_c_str(result, "{\"type\":\"ExportNamedDeclaration\"");
            ec_string_append_c_str(result, ",\"declaration\":null");
            ec_string_append_c_str(result, ",\"specifiers\":[");
            {
                xjs_bool first = xjs_true;
                ect_iterator(xjs_ast_exportspecifierlist) it;
                ect_for(xjs_ast_exportspecifierlist, export_decl->u.as_named->specifiers, it)
                {
                    xjs_ast_exportspecifier_ref specifier = ect_deref(xjs_ast_exportspecifier_ref, it);
                    if (first == xjs_true) first = xjs_false; else ec_string_append_c_str(result, ",");
                    ec_string_append_c_str(result, "{\"type\":\"ExportSpecifier\",\"exported\":");
                    xjs_ast_serialize_identifier(result, specifier->exported);
                    ec_string_append_c_str(result, ",\"local\":");
                    xjs_ast_serialize_identifier(result, specifier->local);
                    {
                        ec_string_append_c_str(result, ",");
                        xjs_ast_serialize_loc(result, &specifier->exported->loc);
                    }
                    ec_string_append_c_str(result, "}");
                }
            }
            ec_string_append_c_str(result, "]");
            ec_string_append_c_str(result, ",\"source\":null");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &export_decl->loc);
            }
            ec_string_append_c_str(result, "}");
            break;
    }

    return 0;
}

static int xjs_ast_serialize_moduleitem( \
        ec_string *result, xjs_ast_moduleitem_ref item)
{
    int ret = 0;

    switch (item->tag)
    {
        case xjs_ast_moduleitem_importdeclaration:
            ret = xjs_ast_serialize_moduleitem_importdecl( \
                    result, item->u.xjs_ast_moduleitem_importdeclaration);
            break;

        case xjs_ast_moduleitem_exportdeclaration:
            ret = xjs_ast_serialize_moduleitem_exportdecl( \
                    result, item->u.xjs_ast_moduleitem_exportdeclaration);
            break;

        case xjs_ast_moduleitem_statementlistitem:
            ret = xjs_ast_serialize_statementlistitem( \
                    result, item->u.xjs_ast_moduleitem_statementlistitem);
            break;
    }

    return ret;
}

static int xjs_ast_serialize_program_script( \
        ec_string *result, xjs_ast_program_ref program)
{
    xjs_ast_statementlist_ref body = program->u.xjs_ast_program_script.body;

    {
        xjs_ast_statementlistitem_ref item = NULL;
        xjs_bool first = xjs_true;
        ect_iterator(xjs_ast_statementlist) it;
        ect_for(xjs_ast_statementlist, body, it)
        {
            if (first) first = xjs_false; else { ec_string_push_back(result, ','); }

            item = ect_deref(xjs_ast_statementlistitem_ref, it);
            if (xjs_ast_serialize_statementlistitem(result, item) != 0)
            { return -1; }
        }
    }

    return 0;
}

static int xjs_ast_serialize_program_module( \
        ec_string *result, xjs_ast_program_ref program)
{
    xjs_ast_moduleitemlist_ref body = program->u.xjs_ast_program_module.body;

    {
        xjs_bool first = xjs_true;
        ect_iterator(xjs_ast_moduleitemlist) it;
        ect_for(xjs_ast_moduleitemlist, body, it)
        {
            xjs_ast_moduleitem_ref item = NULL;
            if (first) first = xjs_false; else { ec_string_push_back(result, ','); }
            item = ect_deref(xjs_ast_moduleitem_ref, it);
            if (xjs_ast_serialize_moduleitem(result, item) != 0)
            { return -1; }
        }
    }

    return 0;
}

static int xjs_ast_serialize_program( \
        ec_string *result, xjs_ast_program_ref program)
{
    int ret = 0;

    ec_string_append_c_str(result, "{\"type\":\"Program\",\"body\":[");
    switch (program->tag)
    {
        case xjs_ast_program_module:
            ret = xjs_ast_serialize_program_module(result, program);
            ec_string_append_c_str(result, "],\"sourceType\":\"module\"");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &program->u.xjs_ast_program_module.loc);
            }
            ec_string_append_c_str(result, "}");
            break;

        case xjs_ast_program_script:
            ret = xjs_ast_serialize_program_script(result, program);
            ec_string_append_c_str(result, "],\"sourceType\":\"script\"");
            {
                ec_string_append_c_str(result, ",");
                xjs_ast_serialize_loc(result, &program->u.xjs_ast_program_script.loc);
            }
            ec_string_append_c_str(result, "}");
            break;
    }

    return ret;
}

/* AST Printer */
int xjs_astprinter_start( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ast_program_ref ast)
{
    int ret = 0;
    ec_string *result = NULL;

    XJS_VNZ_ERROR_MEM(result = ec_string_new(), err);

    if (xjs_ast_serialize_program(result, ast) != 0)
    { goto fail; }

    *result_out = result;

    goto done;
fail:
    ret = -1;
    ec_delete(result); result = NULL;
done:
    return ret;
}

