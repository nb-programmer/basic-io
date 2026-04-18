#pragma once

#include "ast.h"
#include "basic_program.h"

#include <data_structures/stack.h>

/* BASIC interpreter */

typedef enum
{
	KW_DO_NOTHING,
	KW_JMP_ONLY,
	KW_JMP_AND_RET_NEXT,
	KW_JMP_AND_RET_CURR
} KeywordAction;

typedef struct
{
	ASTNodeData value;
	StringLiteral name;
} BASICVariable;

typedef struct
{
	BASICProgram *program;
	int halt;
	BASICVariable *variables;
	int var_count;
	StackNode *traverse_stack;
} BASICRuntime;

typedef ASTNodeData (*basic_function)(BASICRuntime *runtime, ASTNode *args);


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

// Keyword handlers
KeywordAction basic_eval_kw_if(BASICRuntime *runtime, ASTNode *pc, ASTNode **nextpc);
KeywordAction basic_eval_kw_while(BASICRuntime *runtime, ASTNode *pc, ASTNode **nextpc);
