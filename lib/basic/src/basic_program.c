#include "basic/basic.h"

// Standard libraries
#include <stdio.h>
#include <stdlib.h>

BASICProgram *basic_create_program()
{
	BASICProgram *program;
	program = (BASICProgram *)malloc(sizeof(BASICProgram));
	if (program == NULL)
	{
		fprintf(stderr, "Failed to create a BASIC program object\n");
		return NULL;
	}

	program->program_sequence = ast_create_node();
	if (program->program_sequence == NULL)
	{
		free(program);
		return NULL;
	}

	program->program_sequence->type = AST_PROGRAM_SEQUENCE;
	program->program_sequence->data.token.generic = 0;
	program->program_source = NULL;
	program->program_tokens.tokens = NULL;
	program->program_tokens.tokens_length = 0;
	return program;
}

void basic_clear_program(BASICProgram *program)
{
	// Delete each child node and it's children recursively
	ast_delete_children_cascade(program->program_sequence);
	program->program_sequence->next = NULL;
	program->program_sequence->child = NULL;
}

void basic_destroy_program(BASICProgram *program)
{
	// Delete all the nodes in the program sequence
	basic_clear_program(program);
	// Delete the sequence node itself
	ast_delete_node(program->program_sequence);
	// Delete Parse tree data
	if (program->program_tokens.tokens != NULL)
		free(program->program_tokens.tokens);
	// Finally delete the program object
	free(program);
}
