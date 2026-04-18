#pragma once

#include "ast.h"
#include "basic_token.h"

/* Basic Program */

typedef struct
{
	char *program_source;
	BASICTokenParseList program_tokens;
	ASTNode *program_sequence;
	// Map between line number and which instruction to execute on that line. For non-linear control flow
	// BASICLineNode program_line_instruction;
} BASICProgram;

BASICProgram *basic_create_program();
void basic_clear_program(BASICProgram *program);
void basic_destroy_program(BASICProgram *program);
