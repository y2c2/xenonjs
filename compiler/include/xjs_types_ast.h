/* XenonJS : Types : AST
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TYPES_AST_H
#define XJS_TYPES_AST_H

#include <ec_enum.h>
#include <ec_list.h>
#include <ec_string.h>

struct xjs_ast_expression;
typedef struct xjs_ast_expression *xjs_ast_expression_ref;

struct xjs_ast_blockstatement;
typedef struct xjs_ast_blockstatement xjs_ast_blockstatement;
typedef struct xjs_ast_blockstatement *xjs_ast_blockstatement_ref;

struct xjs_ast_statement;
typedef struct xjs_ast_statement *xjs_ast_statement_ref;
struct xjs_ast_statementlistitem;
typedef struct xjs_ast_statementlistitem *xjs_ast_statementlistitem_ref;
struct xjs_ast_statementlist_container;
typedef struct xjs_ast_statementlist_container xjs_ast_statementlist;
typedef struct xjs_ast_statementlist_container *xjs_ast_statementlist_ref;
struct xjs_ast_expressionlist_container;
typedef struct xjs_ast_expressionlist_container xjs_ast_expressionlist;
typedef struct xjs_ast_expressionlist_container *xjs_ast_expressionlist_ref;

/* Location */
struct xjs_opaque_ast_loc
{
    struct
    {
         int ln, col;
    } start, end;
};
typedef struct xjs_opaque_ast_loc xjs_ast_loc;
typedef struct xjs_opaque_ast_loc *xjs_ast_loc_ref;

/* Range */
struct xjs_opaque_ast_range
{
    int start, end;
};
typedef struct xjs_opaque_ast_range xjs_ast_range;
typedef struct xjs_opaque_ast_range *xjs_ast_range_ref;

/* AST : Identifier */
struct xjs_opaque_ast_identifier
{
    ec_string *name;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_identifier xjs_ast_identifier;
typedef struct xjs_opaque_ast_identifier *xjs_ast_identifier_ref;

/* AST : Literal */
typedef enum
{
    xjs_ast_literal_unknown,
    xjs_ast_literal_undefined,
    xjs_ast_literal_null,
    xjs_ast_literal_boolean,
    xjs_ast_literal_number,
    xjs_ast_literal_string,
} xjs_ast_literal_type;
struct xjs_opaque_ast_literal
{
    xjs_ast_literal_type type;
    struct
    {
        double as_number;
        ec_bool as_bool;
        ec_string *as_string;
    } value;
    ec_string *raw;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_literal xjs_ast_literal;
typedef struct xjs_opaque_ast_literal *xjs_ast_literal_ref;

/* AST : Parameter */
struct xjs_opaque_ast_parameter
{
    xjs_ast_identifier_ref id;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_parameter xjs_ast_parameter;
typedef struct xjs_opaque_ast_parameter *xjs_ast_parameter_ref;

/* AST : ParameterList */
ect_list_declare(xjs_ast_parameterlist, xjs_ast_parameter_ref);
typedef xjs_ast_parameterlist *xjs_ast_parameterlist_ref;

/* AST : Property */
struct xjs_opaque_ast_property
{
    xjs_ast_expression_ref key;
    xjs_ast_expression_ref value;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_property xjs_ast_property;
typedef struct xjs_opaque_ast_property *xjs_ast_property_ref;

/* AST : PropertyList */
ect_list_declare(xjs_ast_propertylist, xjs_ast_property_ref);
typedef xjs_ast_propertylist *xjs_ast_propertylist_ref;

/* AST : Expression : Function */
struct xjs_opaque_ast_functionexpression
{
    ec_string *id;
    xjs_ast_parameterlist_ref params;
    /* xjs_ast_statementlist_ref body; */
    xjs_ast_statementlistitem_ref body;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_functionexpression xjs_ast_functionexpression;
typedef struct xjs_opaque_ast_functionexpression *xjs_ast_functionexpression_ref;

/* AST : Expression : Arrow Function */
typedef enum
{
    xjs_opaque_ast_arrowfunctionexpression_body_blockstmt,
    xjs_opaque_ast_arrowfunctionexpression_body_expr
} xjs_opaque_ast_arrowfunctionexpression_body_type;

struct xjs_opaque_ast_arrowfunctionexpression
{
    xjs_ast_parameterlist_ref params;
    struct
    {
        xjs_opaque_ast_arrowfunctionexpression_body_type type;
        union
        {
            xjs_ast_expression_ref as_expr;
            xjs_ast_statementlistitem_ref as_blockstmt;
        } u;
    } body;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_arrowfunctionexpression xjs_ast_arrowfunctionexpression;
typedef struct xjs_opaque_ast_arrowfunctionexpression *xjs_ast_arrowfunctionexpression_ref;

/* AST : Expression : Object */
struct xjs_opaque_ast_objectexpression
{
    xjs_ast_propertylist_ref properties;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_objectexpression xjs_ast_objectexpression;
typedef struct xjs_opaque_ast_objectexpression *xjs_ast_objectexpression_ref;

/* AST : Expression : Array */
struct xjs_opaque_ast_arrayexpression
{
    xjs_ast_expressionlist_ref elements;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_arrayexpression xjs_ast_arrayexpression;
typedef struct xjs_opaque_ast_arrayexpression *xjs_ast_arrayexpression_ref;

/* AST : Expression : Call */
struct xjs_opaque_ast_callexpression
{
    xjs_ast_expression_ref callee;
    xjs_ast_expressionlist_ref arguments;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_callexpression xjs_ast_callexpression;
typedef struct xjs_opaque_ast_callexpression *xjs_ast_callexpression_ref;

/* AST : Expression : Update */
struct xjs_opaque_ast_updateexpression
{
    ec_string *op;
    xjs_ast_expression_ref argument;
    ec_bool prefix;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_updateexpression xjs_ast_updateexpression;
typedef struct xjs_opaque_ast_updateexpression *xjs_ast_updateexpression_ref;

/* AST : Expression : Unary */
struct xjs_opaque_ast_unaryexpression
{
    ec_string *op;
    xjs_ast_expression_ref argument;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_unaryexpression xjs_ast_unaryexpression;
typedef struct xjs_opaque_ast_unaryexpression *xjs_ast_unaryexpression_ref;

/* AST : Expression : Assignment */
struct xjs_opaque_ast_assignmentexpression
{
    ec_string *op;
    xjs_ast_expression_ref left, right;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_assignmentexpression xjs_ast_assignmentexpression;
typedef struct xjs_opaque_ast_assignmentexpression *xjs_ast_assignmentexpression_ref;

/* AST : Expression : Binary */
struct xjs_opaque_ast_binaryexpression
{
    ec_string *op;
    xjs_ast_expression_ref left, right;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_binaryexpression xjs_ast_binaryexpression;
typedef struct xjs_opaque_ast_binaryexpression *xjs_ast_binaryexpression_ref;

/* AST : Expression : Logical */
struct xjs_opaque_ast_logicalexpression
{
    ec_string *op;
    xjs_ast_expression_ref left, right;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_logicalexpression xjs_ast_logicalexpression;
typedef struct xjs_opaque_ast_logicalexpression *xjs_ast_logicalexpression_ref;

/* AST : Expression : Member */
struct xjs_opaque_ast_memberexpression
{
    ec_bool computed;
    xjs_ast_expression_ref object, property;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_memberexpression xjs_ast_memberexpression;
typedef struct xjs_opaque_ast_memberexpression *xjs_ast_memberexpression_ref;

/* AST : Expression : New */
struct xjs_opaque_ast_newexpression
{
    xjs_ast_expression_ref callee;
    xjs_ast_expressionlist_ref arguments;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_newexpression xjs_ast_newexpression;
typedef struct xjs_opaque_ast_newexpression *xjs_ast_newexpression_ref;

/* AST : Expression : Conditional */
struct xjs_opaque_ast_conditionalexpression
{
    xjs_ast_expression_ref test;
    xjs_ast_expression_ref consequent;
    xjs_ast_expression_ref alternate;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_conditionalexpression xjs_ast_conditionalexpression;
typedef struct xjs_opaque_ast_conditionalexpression *xjs_ast_conditionalexpression_ref;

/* AST : Expression : This */
struct xjs_opaque_ast_thisexpression
{
    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_thisexpression xjs_ast_thisexpression;
typedef struct xjs_opaque_ast_thisexpression *xjs_ast_thisexpression_ref;

/* AST : Expression */
ect_enum_declare(xjs_ast_expression, \
        xjs_ast_expression_literal, \
        xjs_ast_expression_identifier, \
        xjs_ast_expression_unaryexpression, \
        xjs_ast_expression_binaryexpression, \
        xjs_ast_expression_logicalexpression, \
        xjs_ast_expression_assignmentexpression, \
        xjs_ast_expression_updateexpression, \
        xjs_ast_expression_callexpression, \
        xjs_ast_expression_functionexpression, \
        xjs_ast_expression_arrowfunctionexpression, \
        xjs_ast_expression_objectexpression, \
        xjs_ast_expression_arrayexpression, \
        xjs_ast_expression_memberexpression, \
        xjs_ast_expression_newexpression, \
        xjs_ast_expression_conditionalexpression, \
        xjs_ast_expression_thisexpression);
ect_enum_declare_begin(xjs_ast_expression)
    xjs_ast_literal_ref xjs_ast_expression_literal;
    xjs_ast_identifier_ref xjs_ast_expression_identifier;
    xjs_ast_unaryexpression_ref xjs_ast_expression_unaryexpression;
    xjs_ast_binaryexpression_ref xjs_ast_expression_binaryexpression;
    xjs_ast_logicalexpression_ref xjs_ast_expression_logicalexpression;
    xjs_ast_assignmentexpression_ref xjs_ast_expression_assignmentexpression;
    xjs_ast_updateexpression_ref xjs_ast_expression_updateexpression;
    xjs_ast_callexpression_ref xjs_ast_expression_callexpression;
    xjs_ast_functionexpression_ref xjs_ast_expression_functionexpression;
    xjs_ast_arrowfunctionexpression_ref xjs_ast_expression_arrowfunctionexpression;
    xjs_ast_objectexpression_ref xjs_ast_expression_objectexpression;
    xjs_ast_arrayexpression_ref xjs_ast_expression_arrayexpression;
    xjs_ast_memberexpression_ref xjs_ast_expression_memberexpression;
    xjs_ast_newexpression_ref xjs_ast_expression_newexpression;
    xjs_ast_conditionalexpression_ref xjs_ast_expression_conditionalexpression;
    xjs_ast_thisexpression_ref xjs_ast_expression_thisexpression;
ect_enum_declare_end();

/* AST : ExpressionList */
ect_list_declare(xjs_ast_expressionlist, xjs_ast_expression_ref);
typedef xjs_ast_expressionlist *xjs_ast_expressionlist_ref;

/* AST : VariableDeclarator */
ect_enum_declare(xjs_ast_variabledeclarator_id, \
        xjs_ast_variabledeclarator_id_identifier, \
        xjs_ast_variabledeclarator_id_bindingpattern);
ect_enum_declare_begin(xjs_ast_variabledeclarator_id)
    xjs_ast_identifier_ref xjs_ast_variabledecorator_identifier;
    void *xjs_ast_variabledecorator_bindingpattern;
ect_enum_declare_end();
typedef xjs_ast_variabledeclarator_id *xjs_ast_variabledeclarator_id_ref;

struct xjs_opaque_ast_variabledeclarator
{
    xjs_ast_variabledeclarator_id_ref id;
    xjs_ast_expression_ref init;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_variabledeclarator xjs_ast_variabledeclarator;
typedef struct xjs_opaque_ast_variabledeclarator *xjs_ast_variabledeclarator_ref;

/* AST : VariableDeclaratorList */
ect_list_declare(xjs_ast_variabledeclaratorlist, xjs_ast_variabledeclarator_ref);
typedef xjs_ast_variabledeclaratorlist *xjs_ast_variabledeclaratorlist_ref;

/* AST : VariableDeclaration */
typedef enum
{
    xjs_ast_variabledeclaration_kind_var,
    xjs_ast_variabledeclaration_kind_const,
    xjs_ast_variabledeclaration_kind_let,
} xjs_ast_variabledeclaration_kind;
struct xjs_opaque_ast_variabledeclaration
{
    xjs_ast_variabledeclaration_kind kind;
    xjs_ast_variabledeclaratorlist_ref declarations;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_variabledeclaration xjs_ast_variabledeclaration;
typedef struct xjs_opaque_ast_variabledeclaration *xjs_ast_variabledeclaration_ref;

/* AST : Declaration */
ect_enum_declare(xjs_ast_declaration, \
        xjs_ast_declaration_classdeclaration, \
        xjs_ast_declaration_functiondeclaration, \
        xjs_ast_declaration_variabledeclaration);
ect_enum_declare_begin(xjs_ast_declaration)
    struct { int dummy; } xjs_ast_declaration_classdeclaration;
    struct { int dummy; } xjs_ast_declaration_functiondeclaration;
    xjs_ast_variabledeclaration_ref xjs_ast_declaration_variabledeclaration;
ect_enum_declare_end();
typedef struct xjs_ast_declaration *xjs_ast_declaration_ref;

/* AST : Expression Statement */
struct xjs_ast_expressionstatement
{
    xjs_ast_expression_ref expression;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_expressionstatement xjs_ast_expressionstatement;
typedef struct xjs_ast_expressionstatement *xjs_ast_expressionstatement_ref;

/* AST : Empty Statement */

/* AST : Block Statement */
struct xjs_ast_blockstatement
{
    xjs_ast_statementlist_ref body;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_blockstatement xjs_ast_blockstatement;
typedef struct xjs_ast_blockstatement *xjs_ast_blockstatement_ref;

/* AST : Inspect Statement */
struct xjs_ast_inspectstatement
{
    xjs_ast_expression_ref argument;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_inspectstatement xjs_ast_inspectstatement;
typedef struct xjs_ast_inspectstatement *xjs_ast_inspectstatement_ref;

/* AST : Return Statement */
struct xjs_ast_returnstatement
{
    xjs_ast_expression_ref argument;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_returnstatement xjs_ast_returnstatement;
typedef struct xjs_ast_returnstatement *xjs_ast_returnstatement_ref;

/* AST : Break Statement */
struct xjs_ast_breakstatement
{
    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_breakstatement xjs_ast_breakstatement;
typedef struct xjs_ast_breakstatement *xjs_ast_breakstatement_ref;

/* AST : Continue Statement */
struct xjs_ast_continuestatement
{
    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_continuestatement xjs_ast_continuestatement;
typedef struct xjs_ast_continuestatement *xjs_ast_continuestatement_ref;

/* AST : If Statement */
struct xjs_ast_ifstatement
{
    xjs_ast_expression_ref test;
    xjs_ast_statementlistitem_ref consequent, alternate;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_ifstatement xjs_ast_ifstatement;
typedef struct xjs_ast_ifstatement *xjs_ast_ifstatement_ref;

/* AST : While Statement */
struct xjs_ast_whilestatement
{
    xjs_ast_expression_ref test;
    xjs_ast_statementlistitem_ref body;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_whilestatement xjs_ast_whilestatement;
typedef struct xjs_ast_whilestatement *xjs_ast_whilestatement_ref;

/* AST : Do Statement */
struct xjs_ast_dostatement
{
    xjs_ast_statementlistitem_ref body;
    xjs_ast_expression_ref test;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_dostatement xjs_ast_dostatement;
typedef struct xjs_ast_dostatement *xjs_ast_dostatement_ref;

/* AST : For Statement */

typedef enum
{
    xjs_ast_forstatement_init_expr,
    xjs_ast_forstatement_init_vardecl,
    xjs_ast_forstatement_init_null,
} xjs_ast_forstatement_init_type;

typedef enum
{
    xjs_ast_forstatement_test_expr,
    xjs_ast_forstatement_test_null,
} xjs_ast_forstatement_test_type;

typedef enum
{
    xjs_ast_forstatement_update_expr,
    xjs_ast_forstatement_update_null,
} xjs_ast_forstatement_update_type;

struct xjs_ast_forstatement
{
    struct
    {
        xjs_ast_forstatement_init_type type;
        union
        {
            xjs_ast_expression_ref as_expr;
            xjs_ast_variabledeclaration_ref as_vardecl;
        } u;
    } init;
    struct
    {
        xjs_ast_forstatement_test_type type;
        union
        {
            xjs_ast_expression_ref as_expr;
        } u;
    } test;
    struct
    {
        xjs_ast_forstatement_update_type type;
        union
        {
            xjs_ast_expression_ref as_expr;
        } u;
    } update;

    xjs_ast_statementlistitem_ref body;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_forstatement xjs_ast_forstatement;
typedef struct xjs_ast_forstatement *xjs_ast_forstatement_ref;

/* AST : Empty Statement */

struct xjs_ast_emptystatement
{
    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_ast_emptystatement xjs_ast_emptystatement;
typedef struct xjs_ast_emptystatement *xjs_ast_emptystatement_ref;

/* AST : Statement */
ect_enum_declare(xjs_ast_statement, \
        xjs_ast_statement_expressionstatement, \
        xjs_ast_statement_emptystatement, \
        xjs_ast_statement_blockstatement, \
        xjs_ast_statement_inspectstatement, \
        xjs_ast_statement_returnstatement, \
        xjs_ast_statement_breakstatement, \
        xjs_ast_statement_continuestatement, \
        xjs_ast_statement_ifstatement, \
        xjs_ast_statement_whilestatement, \
        xjs_ast_statement_dostatement, \
        xjs_ast_statement_forstatement);
ect_enum_declare_begin(xjs_ast_statement)
    xjs_ast_expressionstatement_ref xjs_ast_statement_expressionstatement;
    xjs_ast_emptystatement_ref xjs_ast_statement_emptystatement;
    xjs_ast_blockstatement_ref xjs_ast_statement_blockstatement;
    xjs_ast_returnstatement_ref xjs_ast_statement_returnstatement;
    xjs_ast_breakstatement_ref xjs_ast_statement_breakstatement;
    xjs_ast_continuestatement_ref xjs_ast_statement_continuestatement;
    xjs_ast_inspectstatement_ref xjs_ast_statement_inspectstatement;
    xjs_ast_ifstatement_ref xjs_ast_statement_ifstatement;
    xjs_ast_whilestatement_ref xjs_ast_statement_whilestatement;
    xjs_ast_dostatement_ref xjs_ast_statement_dostatement;
    xjs_ast_forstatement_ref xjs_ast_statement_forstatement;
ect_enum_declare_end();

/* AST : StatementListItem */
ect_enum_declare(xjs_ast_statementlistitem, \
        xjs_ast_statementlistitem_declaration, \
        xjs_ast_statementlistitem_statement);
ect_enum_declare_begin(xjs_ast_statementlistitem)
    xjs_ast_declaration_ref xjs_ast_statementlistitem_declaration;
    xjs_ast_statement_ref xjs_ast_statementlistitem_statement;
ect_enum_declare_end();

/* AST : StatementList */
ect_list_declare(xjs_ast_statementlist, xjs_ast_statementlistitem_ref);

/* AST : ImportSpecifier */
typedef enum
{
    xjs_ast_importspecifier_type_importspecifier,
    xjs_ast_importspecifier_type_importdefaultspecifier,
    xjs_ast_importspecifier_type_importnamespacespecifier,
} xjs_ast_importspecifier_type;
struct xjs_opaque_ast_importspecifier
{
    xjs_ast_importspecifier_type type;
    union
    {
        struct
        {
            xjs_ast_identifier_ref local, imported;
        } importspecifier;
        struct
        {
            xjs_ast_identifier_ref local;
        } importdefaultspecifier;
    } u;
    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_importspecifier xjs_ast_importspecifier;
typedef struct xjs_opaque_ast_importspecifier *xjs_ast_importspecifier_ref;

/* AST : ImportDefaultSpecifier */

/* AST : ImportSpecifierList */
ect_list_declare(xjs_ast_importspecifierlist, xjs_ast_importspecifier_ref);
typedef xjs_ast_importspecifierlist *xjs_ast_importspecifierlist_ref;

/* AST : ImportDeclaration */
struct xjs_opaque_ast_importdeclaration
{
    xjs_ast_importspecifierlist_ref specifiers;
    xjs_ast_literal_ref source;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_importdeclaration xjs_ast_importdeclaration;
typedef struct xjs_opaque_ast_importdeclaration *xjs_ast_importdeclaration_ref;

/* AST : ExportSpecifier */
typedef enum
{
    xjs_ast_exportspecifier_type_exportspecifier,
    xjs_ast_exportspecifier_type_exportdefaultspecifier,
    xjs_ast_exportspecifier_type_exportnamespacespecifier,
} xjs_ast_exportspecifier_type;
struct xjs_opaque_ast_exportspecifier
{
    xjs_ast_exportspecifier_type type;
    xjs_ast_identifier_ref local, exported;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_exportspecifier xjs_ast_exportspecifier;
typedef struct xjs_opaque_ast_exportspecifier *xjs_ast_exportspecifier_ref;

/* AST : ExportSpecifierList */
ect_list_declare(xjs_ast_exportspecifierlist, xjs_ast_exportspecifier_ref);
typedef xjs_ast_exportspecifierlist *xjs_ast_exportspecifierlist_ref;

/* AST : ExportDefaultDeclaration */
typedef enum
{
    xjs_opaque_ast_exportdefaultdeclaration_type_identifier,
    xjs_opaque_ast_exportdefaultdeclaration_type_expression,
} xjs_opaque_ast_exportdefaultdeclaration_type;

struct xjs_opaque_ast_exportdefaultdeclaration
{
    xjs_opaque_ast_exportdefaultdeclaration_type type;
    union
    {
        xjs_ast_identifier_ref as_identifier;
        xjs_ast_expression_ref as_expression;
    } declaration;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_exportdefaultdeclaration xjs_ast_exportdefaultdeclaration;
typedef struct xjs_opaque_ast_exportdefaultdeclaration *xjs_ast_exportdefaultdeclaration_ref;

/* AST : ExportNamedDeclaration */
struct xjs_opaque_ast_exportnameddeclaration_ref
{
    xjs_ast_exportspecifierlist_ref specifiers;
    xjs_ast_literal_ref source;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_exportnameddeclaration_ref xjs_ast_exportnameddeclaration;
typedef struct xjs_opaque_ast_exportnameddeclaration_ref *xjs_ast_exportnameddeclaration_ref;

typedef enum
{
    xjs_ast_exportdeclaration_type_unknown,
    xjs_ast_exportdeclaration_type_all,
    xjs_ast_exportdeclaration_type_default,
    xjs_ast_exportdeclaration_type_named,
} xjs_ast_exportdeclaration_type;
struct xjs_opaque_ast_exportdeclaration
{
    xjs_ast_exportdeclaration_type type;
    union
    {
        xjs_ast_exportnameddeclaration_ref as_named;
        xjs_ast_exportdefaultdeclaration_ref as_default;
    } u;

    xjs_ast_loc loc;
    xjs_ast_range range;
};
typedef struct xjs_opaque_ast_exportdeclaration xjs_ast_exportdeclaration;
typedef struct xjs_opaque_ast_exportdeclaration *xjs_ast_exportdeclaration_ref;

/* AST : ModuleItem */
ect_enum_declare(xjs_ast_moduleitem, \
        xjs_ast_moduleitem_importdeclaration, \
        xjs_ast_moduleitem_exportdeclaration, \
        xjs_ast_moduleitem_statementlistitem);
ect_enum_declare_begin(xjs_ast_moduleitem)
    xjs_ast_importdeclaration_ref xjs_ast_moduleitem_importdeclaration;
    xjs_ast_exportdeclaration_ref xjs_ast_moduleitem_exportdeclaration;
    xjs_ast_statementlistitem_ref xjs_ast_moduleitem_statementlistitem;
ect_enum_declare_end();
typedef xjs_ast_moduleitem *xjs_ast_moduleitem_ref;

/* AST : ModuleItemList */
ect_list_declare(xjs_ast_moduleitemlist, xjs_ast_moduleitem_ref);
typedef xjs_ast_moduleitemlist *xjs_ast_moduleitemlist_ref;

/* AST : Program */
ect_enum_declare(xjs_ast_program, \
        xjs_ast_program_script, \
        xjs_ast_program_module);
ect_enum_declare_begin(xjs_ast_program)
    struct { xjs_ast_statementlist_ref body; xjs_ast_loc loc; xjs_ast_range range; } xjs_ast_program_script;
    struct { xjs_ast_moduleitemlist_ref body; xjs_ast_loc loc; xjs_ast_range range; } xjs_ast_program_module;
ect_enum_declare_end();
typedef struct xjs_ast_program *xjs_ast_program_ref;

#endif

