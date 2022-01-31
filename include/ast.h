#ifndef _AST_H
#define _AST_H

typedef char StringLiteral[101];

/* Abstract Syntax Tree */

typedef union {
    float flt;
    int num;
    StringLiteral str;
} Literal;

typedef enum {
    OP_NONE,

    //Unary
    OP_NOT,
    OP_NEGATE,

    //Binary
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_EQ,
    OP_LT,
    OP_GT,
    OP_ASSIGN,

    //Separator, only used during parsing
    OP_OPEN_PAREN,
    OP_CLOSE_PAREN
} ASTOperator;

typedef enum {
    OPTYPE_NONE,
    OPTYPE_UNARY,
    OPTYPE_BINARY,
    OPTYPE_TERNARY
} ASTOperatorType;

typedef union {
    int generic;
    //For keyword / operation / function call
    StringLiteral kw;

    //For a variable name
    StringLiteral variable_name;

    //token_type is used for following

    //For a literal number/string
    Literal literal;
    //For ALU operation
    ASTOperator op;
} ASTData;

typedef enum {
    DTYPE_NONE,
    DTYPE_STR,
    DTYPE_NUM,
    DTYPE_FLT,
    DTYPE_SYMB
} ASTDType;

typedef struct {
    ASTData token;
    ASTDType token_type;
} ASTNodeData;

typedef enum {
    AST_NONE,
    AST_PROGRAM_SEQUENCE,

    //Operation
    AST_KEYWORD,
    AST_EXPRESSION,
    AST_OPERATION,
    AST_CONDITION,

    //Data
    AST_IMMEDIATE,
    AST_VARIABLE,

    //Call to function
    AST_FUNC_CALL,
} ASTNodeType;

typedef struct _ast_node {
    ASTNodeType type;
    ASTNodeData data;

    struct _ast_node *next;
    struct _ast_node *child;
} ASTNode;

//Void data
extern ASTNodeData ASTVOID;

void ast_display_level(ASTNode *node, int level);

//Public functions
ASTNode *ast_create_node();
void ast_append_child(ASTNode *parent, ASTNode *node);
void ast_delete_node(ASTNode *node);
void ast_delete_children_cascade(ASTNode *root);
void ast_display(ASTNode *node);
void ast_data_as_string(ASTNodeData ast_data, char *buffer);
int ast_data_to_int(ASTNodeData ast_data);
float ast_data_to_flt(ASTNodeData ast_data);
ASTOperatorType ast_get_operator_type(ASTOperator op);

int ast_operator_precedence(ASTOperator op);
int ast_evaluate_unary(ASTOperator op, ASTNodeData operand, ASTNodeData *result);
int ast_evaluate_binary(ASTOperator op, ASTNodeData a, ASTNodeData b, ASTNodeData *result);
int ast_get_greater(ASTNodeData a, ASTNodeData b, ASTNodeData *result);
int ast_get_lesser(ASTNodeData a, ASTNodeData b, ASTNodeData *result);

#endif
