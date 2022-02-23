/* XenonJS : AST
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_AST_H
#define XJS_AST_H

#include <ec_list.h>
#include <ec_enum.h>
#include "xjs_types.h"

/* AST : Identifier */
xjs_ast_identifier_ref xjs_ast_identifier_new(ec_string *name);

/* AST : Identifier : Id */
xjs_ast_variabledeclarator_id_ref
xjs_ast_variabledeclarator_id_identifier_new(xjs_ast_identifier_ref id);

/* AST : Literal */
xjs_ast_literal_ref xjs_ast_literal_new(void);
void xjs_ast_literal_value_set_undefined(xjs_ast_literal_ref r);
void xjs_ast_literal_value_set_null(xjs_ast_literal_ref r);
void xjs_ast_literal_value_set_boolean(xjs_ast_literal_ref r, ec_bool value);
void xjs_ast_literal_value_set_number(xjs_ast_literal_ref r, double value);
void xjs_ast_literal_value_set_string(xjs_ast_literal_ref r, ec_string *value);
void xjs_ast_literal_raw_set(xjs_ast_literal_ref r, ec_string *raw);

/* AST : Parameter */
xjs_ast_parameter_ref xjs_ast_parameter_new(xjs_ast_identifier_ref id);

/* AST : Property */
xjs_ast_property_ref xjs_ast_property_new(void);
void xjs_ast_property_set_key(xjs_ast_property_ref property, xjs_ast_expression_ref key);
void xjs_ast_property_set_value(xjs_ast_property_ref property, xjs_ast_expression_ref value);

/* AST : VariableDeclarator */
xjs_ast_variabledeclarator_ref xjs_ast_variabledeclarator_new(void);
void xjs_ast_variabledeclarator_id_set( \
        xjs_ast_variabledeclarator_ref r, \
        xjs_ast_variabledeclarator_id_ref decl_id);
xjs_ast_variabledeclarator_id_ref xjs_ast_variabledeclarator_id_get( \
        xjs_ast_variabledeclarator_ref r);
void xjs_ast_variabledeclarator_init_set( \
        xjs_ast_variabledeclarator_ref r, \
        xjs_ast_expression_ref expr);
xjs_ast_expression_ref xjs_ast_variabledeclarator_init_get( \
        xjs_ast_variabledeclarator_ref r);

/* AST : VariableDeclaration */
xjs_ast_variabledeclaration_ref xjs_ast_variabledeclaration_new(void);
void xjs_ast_variabledeclaration_kind_set( \
        xjs_ast_variabledeclaration_ref r, \
        xjs_ast_variabledeclaration_kind kind);
xjs_ast_variabledeclaration_kind
xjs_ast_variabledeclaration_kind_get( \
        xjs_ast_variabledeclaration_ref r);

/* AST : Declaration */
xjs_ast_declaration_ref xjs_ast_declaration_variabledeclaration_new(void);

/* AST : UpdateExpression */
xjs_ast_updateexpression_ref xjs_ast_updateexpression_new(void);
void xjs_ast_updateexpression_op_set( \
        xjs_ast_updateexpression_ref r, \
        ec_string *op);
void xjs_ast_updateexpression_argument_set( \
        xjs_ast_updateexpression_ref r, \
        xjs_ast_expression_ref argument);
void xjs_ast_updateexpression_prefix_set( \
        xjs_ast_updateexpression_ref r, \
        ec_bool prefix);

/* AST : UnaryExpression */
xjs_ast_unaryexpression_ref xjs_ast_unaryexpression_new(void);
void xjs_ast_unaryexpression_op_set( \
        xjs_ast_unaryexpression_ref r, \
        ec_string *op);
void xjs_ast_unaryexpression_argument_set( \
        xjs_ast_unaryexpression_ref r, \
        xjs_ast_expression_ref argument);

/* AST : AssignmentExpression */
xjs_ast_assignmentexpression_ref xjs_ast_assignmentexpression_new(void);
void xjs_ast_assignmentexpression_op_set( \
        xjs_ast_assignmentexpression_ref r, \
        ec_string *op);
void xjs_ast_assignmentexpression_left_set( \
        xjs_ast_assignmentexpression_ref r, \
        xjs_ast_expression_ref left);
void xjs_ast_assignmentexpression_right_set( \
        xjs_ast_assignmentexpression_ref r, \
        xjs_ast_expression_ref right);

/* AST : BinaryExpression */
xjs_ast_binaryexpression_ref xjs_ast_binaryexpression_new(void);
void xjs_ast_binaryexpression_op_set( \
        xjs_ast_binaryexpression_ref r, \
        ec_string *op);
void xjs_ast_binaryexpression_left_set( \
        xjs_ast_binaryexpression_ref r, \
        xjs_ast_expression_ref left);
void xjs_ast_binaryexpression_right_set( \
        xjs_ast_binaryexpression_ref r, \
        xjs_ast_expression_ref right);

/* AST : LogicalExpression */
xjs_ast_logicalexpression_ref xjs_ast_logicalexpression_new(void);
void xjs_ast_logicalexpression_op_set( \
        xjs_ast_logicalexpression_ref r, \
        ec_string *op);
void xjs_ast_logicalexpression_left_set( \
        xjs_ast_logicalexpression_ref r, \
        xjs_ast_expression_ref left);
void xjs_ast_logicalexpression_right_set( \
        xjs_ast_logicalexpression_ref r, \
        xjs_ast_expression_ref right);

/* AST : FunctionExpression */
xjs_ast_functionexpression_ref xjs_ast_functionexpression_new(void);
void xjs_ast_functionexpression_id_set( \
        xjs_ast_functionexpression_ref r, \
        ec_string *id);
int xjs_ast_functionexpression_parameter_push_back( \
        xjs_ast_functionexpression_ref r, \
        xjs_ast_identifier_ref id);
void xjs_ast_functionexpression_body_set( \
        xjs_ast_functionexpression_ref r, \
        xjs_ast_statementlistitem_ref body);

/* AST : ArrowFunctionExpression */
xjs_ast_arrowfunctionexpression_ref xjs_ast_arrowfunctionexpression_new(void);
int xjs_ast_arrowfunctionexpression_parameter_push_back( \
        xjs_ast_arrowfunctionexpression_ref r, \
        xjs_ast_identifier_ref id);
void xjs_ast_arrowfunctionexpression_body_expr_set( \
        xjs_ast_arrowfunctionexpression_ref r, \
        xjs_ast_expression_ref expr);
void xjs_ast_arrowfunctionexpression_body_statement_set( \
        xjs_ast_arrowfunctionexpression_ref r, \
        xjs_ast_statementlistitem_ref blockstmt);

/* AST : ObjectExpression */
xjs_ast_objectexpression_ref xjs_ast_objectexpression_new(void);
int xjs_ast_objectexpression_property_push_back( \
        xjs_ast_objectexpression_ref r, \
        xjs_ast_property_ref p);

/* AST : ArrayExpression */
xjs_ast_arrayexpression_ref xjs_ast_arrayexpression_new(void);
int xjs_ast_arrayexpression_element_push_back( \
        xjs_ast_arrayexpression_ref r, \
        xjs_ast_expression_ref expr);

/* AST : CallExpression */
xjs_ast_callexpression_ref xjs_ast_callexpression_new(void);
void xjs_ast_callexpression_callee_set( \
        xjs_ast_callexpression_ref r, \
        xjs_ast_expression_ref callee);
void xjs_ast_callexpression_arguments_set( \
        xjs_ast_callexpression_ref r, \
        xjs_ast_expressionlist_ref arguments);

/* AST : MemberExpression */
xjs_ast_memberexpression_ref xjs_ast_memberexpression_new(void);
void xjs_ast_memberexpression_computed_set( \
        xjs_ast_memberexpression_ref r, \
        ec_bool computed);
void xjs_ast_memberexpression_object_set( \
        xjs_ast_memberexpression_ref r, \
        xjs_ast_expression_ref object);
void xjs_ast_memberexpression_property_set( \
        xjs_ast_memberexpression_ref r, \
        xjs_ast_expression_ref prop);

/* AST : NewExpression */
xjs_ast_newexpression_ref xjs_ast_newexpression_new(void);
void xjs_ast_newexpression_callee_set( \
        xjs_ast_newexpression_ref r, \
        xjs_ast_expression_ref callee);
void xjs_ast_newexpression_arguments_set( \
        xjs_ast_newexpression_ref r, \
        xjs_ast_expressionlist_ref arguments);

/* AST : ConditionalExpression */
xjs_ast_conditionalexpression_ref xjs_ast_conditionalexpression_new(void);
void xjs_ast_conditionalexpression_test_set( \
        xjs_ast_conditionalexpression_ref r, \
        xjs_ast_expression_ref test);
void xjs_ast_conditionalexpression_consequent_set( \
        xjs_ast_conditionalexpression_ref r, \
        xjs_ast_expression_ref consequent);
void xjs_ast_conditionalexpression_alternate_set( \
        xjs_ast_conditionalexpression_ref r, \
        xjs_ast_expression_ref alternate);

/* AST : ThisExpression */
xjs_ast_thisexpression_ref xjs_ast_thisexpression_new(void);

/* AST : Expression */
xjs_ast_expression_ref xjs_ast_expression_identifier_new(void);
void xjs_ast_expression_identifier_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_identifier_ref id);
xjs_ast_expression_ref xjs_ast_expression_literal_new(void);
void xjs_ast_expression_literal_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_literal_ref lit);
xjs_ast_expression_ref xjs_ast_expression_unaryexpression_new(void);
void xjs_ast_expression_unaryexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_unaryexpression_ref unary_expr);
xjs_ast_expression_ref xjs_ast_expression_binaryexpression_new(void);
void xjs_ast_expression_binaryexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_binaryexpression_ref binary_expr);
xjs_ast_expression_ref xjs_ast_expression_logicalexpression_new(void);
void xjs_ast_expression_logicalexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_logicalexpression_ref logical_expr);
xjs_ast_expression_ref xjs_ast_expression_assignmentexpression_new(void);
void xjs_ast_expression_assignmentexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_assignmentexpression_ref assignment_expr);
xjs_ast_expression_ref xjs_ast_expression_updateexpression_new(void);
void xjs_ast_expression_updateexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_updateexpression_ref update_expr);
xjs_ast_expression_ref xjs_ast_expression_functionexpression_new(void);
void xjs_ast_expression_functionexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_functionexpression_ref function_expr);
xjs_ast_expression_ref xjs_ast_expression_arrowfunctionexpression_new(void);
void xjs_ast_expression_arrowfunctionexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_arrowfunctionexpression_ref arrowfunction_expr);
xjs_ast_expression_ref xjs_ast_expression_objectexpression_new(void);
void xjs_ast_expression_objectexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_objectexpression_ref object_expr);
xjs_ast_expression_ref xjs_ast_expression_arrayexpression_new(void);
void xjs_ast_expression_arrayexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_arrayexpression_ref array_expr);
xjs_ast_expression_ref xjs_ast_expression_callexpression_new(void);
void xjs_ast_expression_callexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_callexpression_ref call_expr);
xjs_ast_expression_ref xjs_ast_expression_memberexpression_new(void);
void xjs_ast_expression_memberexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_memberexpression_ref member_expr);
xjs_ast_expression_ref xjs_ast_expression_newexpression_new(void);
void xjs_ast_expression_newexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_newexpression_ref new_expr);
xjs_ast_expression_ref xjs_ast_expression_conditionalexpression_new(void);
void xjs_ast_expression_conditionalexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_conditionalexpression_ref conditional_expr);
xjs_ast_expression_ref xjs_ast_expression_thisexpression_new(void);
void xjs_ast_expression_thisexpression_set( \
        xjs_ast_expression_ref r, \
        xjs_ast_thisexpression_ref this_expr);

/* AST : EmptyStatement */
xjs_ast_emptystatement_ref xjs_ast_emptystatement_new(void);

/* AST : Block Statement */
xjs_ast_blockstatement_ref xjs_ast_blockstatement_new(void);

/* AST : Inspect Statement */
xjs_ast_inspectstatement_ref xjs_ast_inspectstatement_new(void);

/* AST : Return Statement */
xjs_ast_returnstatement_ref xjs_ast_returnstatement_new(void);

/* AST : Break Statement */
xjs_ast_breakstatement_ref xjs_ast_breakstatement_new(void);

/* AST : Continue Statement */
xjs_ast_continuestatement_ref xjs_ast_continuestatement_new(void);

/* AST : If Statement */
xjs_ast_ifstatement_ref xjs_ast_ifstatement_new(void);

/* AST : While Statement */
xjs_ast_whilestatement_ref xjs_ast_whilestatement_new(void);

/* AST : Do Statement */
xjs_ast_dostatement_ref xjs_ast_dostatement_new(void);

/* AST : For Statement */
xjs_ast_forstatement_ref xjs_ast_forstatement_new(void);

/* AST : Expression Statement */
xjs_ast_expressionstatement_ref xjs_ast_expressionstatement_new(void);

/* AST : Statement */
xjs_ast_statement_ref xjs_ast_statement_emptystatement_new(void);
xjs_ast_statement_ref xjs_ast_statement_expressionstatement_new(void);
void xjs_ast_statement_expressionstatement_expression_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref expr);
xjs_ast_statement_ref xjs_ast_statement_blockstatement_new(void);
void xjs_ast_statement_blockstatement_push_back(xjs_ast_statement_ref r,
        xjs_ast_statementlistitem_ref new_item);
xjs_ast_statement_ref xjs_ast_statement_inspectstatement_new(void);
void xjs_ast_statement_inspectstatement_argument_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref argument);
xjs_ast_statement_ref xjs_ast_statement_returnstatement_new(void);
void xjs_ast_statement_returnstatement_argument_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref argument);
xjs_ast_statement_ref xjs_ast_statement_breakstatement_new(void);
xjs_ast_statement_ref xjs_ast_statement_continuestatement_new(void);
xjs_ast_statement_ref xjs_ast_statement_ifstatement_new(void);
void xjs_ast_statement_ifstatement_test_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref test);
void xjs_ast_statement_ifstatement_consequent_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref consequent);
void xjs_ast_statement_ifstatement_alternate_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref alternate);
xjs_ast_statement_ref xjs_ast_statement_whilestatement_new(void);
void xjs_ast_statement_whilestatement_test_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref test);
void xjs_ast_statement_whilestatement_body_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref body);
xjs_ast_statement_ref xjs_ast_statement_dostatement_new(void);
void xjs_ast_statement_dostatement_body_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref body);
void xjs_ast_statement_dostatement_test_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref test);
xjs_ast_statement_ref xjs_ast_statement_forstatement_new(void);
void xjs_ast_statement_forstatement_init_set_null( \
        xjs_ast_statement_ref r);
void xjs_ast_statement_forstatement_init_set_expr( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref init);
void xjs_ast_statement_forstatement_init_set_vardecl( \
        xjs_ast_statement_ref r, \
        xjs_ast_variabledeclaration_ref vardecl);
void xjs_ast_statement_forstatement_test_set_null( \
        xjs_ast_statement_ref r);
void xjs_ast_statement_forstatement_test_set_expr( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref test);
void xjs_ast_statement_forstatement_update_set_null( \
        xjs_ast_statement_ref r);
void xjs_ast_statement_forstatement_update_set_expr( \
        xjs_ast_statement_ref r, \
        xjs_ast_expression_ref update);
void xjs_ast_statement_forstatement_body_set( \
        xjs_ast_statement_ref r, \
        xjs_ast_statementlistitem_ref body);

/* AST : Statement List Item */
xjs_ast_statementlistitem_ref xjs_ast_statementlistitem_declaration_new(void);
void xjs_ast_statementlistitem_declaration_declaration_set( \
        xjs_ast_statementlistitem_ref r, xjs_ast_declaration_ref decl);
xjs_ast_statementlistitem_ref xjs_ast_statementlistitem_statement_new(void);
void xjs_ast_statementlistitem_statement_statement_set( \
        xjs_ast_statementlistitem_ref r, xjs_ast_statement_ref stmt);

/* AST : ImportSpecifier */
xjs_ast_importspecifier_ref xjs_ast_importspecifier_importspecifier_new(void);
xjs_ast_importspecifier_ref xjs_ast_importspecifier_importdefaultspecifier_new(void);
xjs_ast_importspecifier_ref xjs_ast_importspecifier_importnamespacespecifier_new(void);

/* AST : ImportDeclaration */
xjs_ast_importdeclaration_ref xjs_ast_importdeclaration_new(void);
void xjs_ast_importdeclaration_source_set( \
        xjs_ast_importdeclaration_ref r, \
        xjs_ast_literal_ref source);
void xjs_ast_importdeclaration_specifiers_push_back(xjs_ast_importdeclaration_ref r, \
        xjs_ast_importspecifier_ref new_importspecifier);

/* AST : ExportSpecifier */
xjs_ast_exportspecifier_ref xjs_ast_exportspecifier_new(void);

/* AST : ExportDeclaration */
xjs_ast_exportdefaultdeclaration_ref xjs_ast_exportdefaultdeclaration_identifier_new(void);
xjs_ast_exportdefaultdeclaration_ref xjs_ast_exportdefaultdeclaration_expression_new(void);
xjs_ast_exportdeclaration_ref xjs_ast_exportdeclaration_default_new(void);
xjs_ast_exportnameddeclaration_ref xjs_ast_exportnameddeclaration_new(void);
xjs_ast_exportdeclaration_ref xjs_ast_exportdeclaration_named_new(void);
void xjs_ast_exportdeclaration_default_set( \
        xjs_ast_exportdeclaration_ref r, \
        xjs_ast_exportdefaultdeclaration_ref defaultdecl);
void xjs_ast_exportdeclaration_named_specifiers_push_back(xjs_ast_exportdeclaration_ref r, \
        xjs_ast_exportspecifier_ref new_exportspecifier);

/* AST : ModuleItem */
xjs_ast_moduleitem_ref xjs_ast_moduleitem_importdeclaration_new(void);
void xjs_ast_moduleitem_importdeclaration_set( \
        xjs_ast_moduleitem_ref r, \
        xjs_ast_importdeclaration_ref importdeclaration);
xjs_ast_moduleitem_ref xjs_ast_moduleitem_exportdeclaration_new(void);
void xjs_ast_moduleitem_exportdeclaration_set( \
        xjs_ast_moduleitem_ref r, \
        xjs_ast_exportdeclaration_ref exportdeclaration);
xjs_ast_moduleitem_ref xjs_ast_moduleitem_statementlistitem_new(void);
void xjs_ast_moduleitem_statementlistitem_set( \
        xjs_ast_moduleitem_ref r, \
        xjs_ast_statementlistitem_ref new_item);

/* AST : Program */
xjs_ast_program_ref xjs_ast_program_script_new(void);
void xjs_ast_program_script_push_back( \
        xjs_ast_program_ref program, \
        xjs_ast_statementlistitem_ref new_item);

/* AST : Module */
xjs_ast_program_ref xjs_ast_program_module_new(void);
void xjs_ast_program_module_push_back( \
        xjs_ast_program_ref program, \
        xjs_ast_moduleitem_ref new_item);

#endif

