#pragma once

#include "ast.h"
#include "basic_token.h"
#include "basic_program.h"

// Recognized tokens
extern char *PARSE_KEYWORDS[];
extern int PARSE_KW_COUNT;
extern char *PARSE_BOOLEAN[];
extern char PARSE_OPERATORS[];
extern char PARSE_SEPARATOR[];
extern char PARSE_WS_CHAR[];

// Index of the keyword in the above PARSE_KEYWORDS array
#define KEYWORD_IDX_WHILE 0
#define KEYWORD_IDX_IF 1
#define KEYWORD_IDX_THEN 2
#define KEYWORD_IDX_ELSE 3
#define KEYWORD_IDX_END 4
#define KEYWORD_IDX_GOTO 5

// Parser
int basic_parse_form_expression(BASICParseTree *ptree, ASTNode *root, int parse_from, int parse_to, int *parse_new_pos);
int basic_parse_form_function(BASICParseTree *ptree, ASTNode *root, int parse_from, int parse_to, int *parse_new_pos);

int basic_parse_to_ast_between_level(BASICParseTree *ptree, ASTNode *root, int from, int to, int level, int allow_keyword, int *next_ptr);
int basic_parse_to_ast_between_onlyexpr(BASICParseTree *ptree, ASTNode *root, int from, int to);
int basic_parse_to_ast_between(BASICParseTree *ptree, ASTNode *root, int from, int to);
int basic_parse_to_ast(BASICProgram *program);
