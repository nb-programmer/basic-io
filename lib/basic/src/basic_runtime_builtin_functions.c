#include <stdio.h>

#include "basic/basic.h"
#include "basic/ast.h"

#include <utility/logging/logging.h>
#include <basic_system_interface/system.h>

// Simple print function
ASTNodeData basic_fn_print(BASICRuntime *runtime, ASTNode *args)
{
	ASTNode *arg = args;
	char temp[512] = {0};
	while (arg != NULL)
	{
		ASTNodeData value = basic_evaluate_node(runtime, arg);
		ast_data_as_string(value, temp);
		printf("%s", temp);
		arg = arg->next;
		// Space separated arguments
		if (arg != NULL)
			printf(" ");
	}
	printf("\n");
	fflush(stdout);

	// No return value
	return ASTVOID;
}

// Function to find maximum value from given parameters
ASTNodeData basic_fn_max(BASICRuntime *runtime, ASTNode *args)
{
	ASTNode *arg = args;
	ASTNodeData ret_val = ASTVOID;
	while (arg != NULL)
	{
		ASTNodeData value = basic_evaluate_node(runtime, arg);
		ast_get_greater(ret_val, value, &ret_val);
		arg = arg->next;
	}
	return ret_val;
}

// Function to find minimum value from given parameters
ASTNodeData basic_fn_min(BASICRuntime *runtime, ASTNode *args)
{
	ASTNode *arg = args;
	ASTNodeData ret_val = ASTVOID;
	while (arg != NULL)
	{
		ASTNodeData value = basic_evaluate_node(runtime, arg);
		ast_get_lesser(ret_val, value, &ret_val);
		arg = arg->next;
	}

	return ret_val;
}

// Sleep for given number of seconds (can be fraction)
ASTNodeData basic_fn_sleep(BASICRuntime *runtime, ASTNode *arg)
{
	// Exit if no argument passed
	if (arg == NULL)
		return ASTVOID;
	// Evaluate the expression in the given argument
	ASTNodeData value = basic_evaluate_node(runtime, arg);
	// Convert to float
	float sleep_seconds = ast_data_to_flt(value);
	system_sleep(sleep_seconds);
	return (ASTNodeData){0, DTYPE_NUM};
}

// Convert given integer / float to integer
ASTNodeData basic_fn_toint(BASICRuntime *runtime, ASTNode *arg)
{
	if (arg == NULL)
	{
		lprintf("EXEC", LOGTYPE_ERROR, "Error: Expected one argument to convert to integer, none found\n");
		runtime->halt = 1;
		return ASTVOID;
	}
	ASTNodeData value = basic_evaluate_node(runtime, arg);
	int as_int = ast_data_to_int(value);
	value.token.literal.flt = as_int;
	value.token_type = DTYPE_NUM;
	return value;
}

// Convert given integer / float to float
ASTNodeData basic_fn_toflt(BASICRuntime *runtime, ASTNode *arg)
{
	if (arg == NULL)
	{
		lprintf("EXEC", LOGTYPE_ERROR, "Error: Expected one argument to convert to float, none found\n");
		runtime->halt = 1;
		return ASTVOID;
	}
	ASTNodeData value = basic_evaluate_node(runtime, arg);
	float as_flt = ast_data_to_flt(value);
	value.token.literal.flt = as_flt;
	value.token_type = DTYPE_FLT;
	return value;
}

ASTNodeData basic_fn_rand(BASICRuntime *runtime, ASTNode *arg)
{
	if (arg != NULL)
	{
		lprintf("EXEC", LOGTYPE_ERROR, "Error: Function call does not expect any arguments\n");
		runtime->halt = 1;
		return ASTVOID;
	}
	ASTNodeData random;
	random.token.literal.flt = system_random_float();
	random.token_type = DTYPE_FLT;
	return random;
}

ASTNodeData basic_fn_irand(BASICRuntime *runtime, ASTNode *arg)
{
	if (arg != NULL)
	{
		lprintf("EXEC", LOGTYPE_ERROR, "Error: Function call does not expect any arguments\n");
		runtime->halt = 1;
		return ASTVOID;
	}
	ASTNodeData random;
	random.token.literal.num = system_random_int();
	random.token_type = DTYPE_NUM;
	return random;
}
