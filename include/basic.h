#ifndef _BASIC_H
#define _BASIC_H

/* BASIC Program parser / interpreter common header file */

/**
 * BASIC interpreter steps:
 * 
 * 0. BASIC program source code input
 * 1. BASIC Lexer (Program source to tokens)
 * 2. BASIC Parser (Tokens to AST)
 * 3. Execute AST sequentially (Program run)
 * 
 * */

#include "ast.h"
#include "stack.h"

/* For printing Log messages */
extern unsigned int log_print_mask;

//Log level mask
#define LOGTYPE_DEBUG   (1 << 0)
#define LOGTYPE_INFO    (1 << 1)
#define LOGTYPE_MESSAGE (1 << 2)
#define LOGTYPE_ERROR   (1 << 3)
#define LOGMASK_ALL     0xffff

//Printf modified to allow only selected messages
void lprintf(const char *tag, unsigned int level_mask, const char *format, ...);

/* AST parser (tokenizer) */

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_WHITESPACE,
    TOKEN_OPERATOR,
    TOKEN_SEPARATOR,
    TOKEN_NUM,
    TOKEN_BOOL,
    TOKEN_STRING,
    TOKEN_END
} BASICTokenType;

typedef struct {
    StringLiteral token;
    BASICTokenType token_type;
    char *token_at;
} BASICToken;

//Structure with all the stuff needed to convert a BASIC program to an AST
typedef struct {
    //List of tokens
    BASICToken *tokens;
    int tokens_length;
    //Stack for knowning current scope
} BASICParseTree;

typedef struct {
    char *program_source;
    BASICParseTree program_tokens;
    ASTNode *program_sequence;
    //Map between line number and which instruction to execute on that line. For non-linear control flow
    //BASICLineNode program_line_instruction;
} BASICProgram;

BASICProgram *basic_create_program();
void basic_clear_program(BASICProgram *program);
void basic_destroy_program(BASICProgram *program);

//Recognized tokens
extern char *PARSE_KEYWORDS[];
extern int PARSE_KW_COUNT;
extern char *PARSE_BOOLEAN[];
extern char PARSE_OPERATORS[];
extern char PARSE_SEPARATOR[];
extern char PARSE_WS_CHAR[];

//Index of the keyword in the above PARSE_KEYWORDS array
#define KEYWORD_IDX_WHILE       0
#define KEYWORD_IDX_IF          1
#define KEYWORD_IDX_THEN        2
#define KEYWORD_IDX_ELSE        3
#define KEYWORD_IDX_END         4
#define KEYWORD_IDX_GOTO        5

//Lexer
int basic_tokenize(BASICProgram *program);

//Parser
int basic_parse_form_expression(BASICParseTree *ptree, ASTNode *root, int parse_from, int parse_to, int *parse_new_pos);
int basic_parse_form_function(BASICParseTree *ptree, ASTNode *root, int parse_from, int parse_to, int *parse_new_pos);

int basic_parse_to_ast_between_level(BASICParseTree *ptree, ASTNode *root, int from, int to, int level, int allow_keyword, int *next_ptr);
int basic_parse_to_ast_between_onlyexpr(BASICParseTree *ptree, ASTNode *root, int from, int to);
int basic_parse_to_ast_between(BASICParseTree *ptree, ASTNode *root, int from, int to);
int basic_parse_to_ast(BASICProgram *program);


/* BASIC interpreter */

typedef enum {
    KW_DO_NOTHING,
    KW_JMP_ONLY,
    KW_JMP_AND_RET_NEXT,
    KW_JMP_AND_RET_CURR
} KeywordAction;

typedef struct {
    ASTNodeData value;
    StringLiteral name;
} BASICVariable;

typedef struct {
    BASICProgram *program;
    int halt;
    BASICVariable *variables;
    int var_count;
    StackNode *traverse_stack;
} BASICRuntime;

typedef ASTNodeData(*basic_function)(BASICRuntime *runtime, ASTNode *args);

/* Public functions */
BASICRuntime *basic_create_runtime(BASICProgram *program);
void basic_free_runtime(BASICRuntime *runtime);

ASTNodeData basic_evaluate_node(BASICRuntime *runtime, ASTNode *node);
ASTNodeData basic_execute(BASICRuntime *runtime, ASTNode *pc);

/* Private functions */

BASICVariable *basic_find_variable(BASICRuntime *runtime, char var_name[]);
ASTNodeData basic_get_variable(BASICRuntime *runtime, char var_name[]);
void basic_set_variable(BASICRuntime *runtime, char var_name[], ASTNodeData value);
void basic_var_assignment(BASICRuntime *runtime, ASTNode *args);
void basic_init_constants(BASICRuntime *runtime);
KeywordAction basic_evaluate_keyword_block(BASICRuntime *runtime, ASTNode *pc, ASTNode **nextpc);

//Keyword handlers
KeywordAction basic_eval_kw_if(BASICRuntime *runtime, ASTNode *pc, ASTNode **nextpc);
KeywordAction basic_eval_kw_while(BASICRuntime *runtime, ASTNode *pc, ASTNode **nextpc);

//Built-in functions
ASTNodeData _basic_fn_print(BASICRuntime *runtime, ASTNode *args);
ASTNodeData _basic_fn_max(BASICRuntime *runtime, ASTNode *args);
ASTNodeData _basic_fn_min(BASICRuntime *runtime, ASTNode *args);
ASTNodeData _basic_fn_sleep(BASICRuntime *runtime, ASTNode *arg);
ASTNodeData _basic_fn_toint(BASICRuntime *runtime, ASTNode *arg);
ASTNodeData _basic_fn_toflt(BASICRuntime *runtime, ASTNode *arg);
ASTNodeData _basic_fn_rand(BASICRuntime *runtime, ASTNode *arg);
ASTNodeData _basic_fn_irand(BASICRuntime *runtime, ASTNode *arg);

#endif