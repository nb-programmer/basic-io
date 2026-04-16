// Standard libraries
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <utility/logging/logging.h>

#include <basic/ast.h>
#include <basic/basic.h>

// Simply run the program sequence from the given program
void interpret_basic_program(BASICProgram *program)
{
	StringLiteral buffer;
	BASICRuntime *runtime;

	if (basic_tokenize(program) != 0)
	{
		return;
	}

	if (basic_parse_to_ast(program))
	{
		return;
	}

	runtime = basic_create_runtime(program);

	if (runtime == NULL)
		return;

	basic_execute(runtime, program->program_sequence);

	basic_free_runtime(runtime);
}

void read_basic_program(FILE *basic_program_file, char **program_buffer)
{
	size_t program_buffer_size;
	long file_size;

	fseek(basic_program_file, 0, SEEK_END);
	file_size = ftell(basic_program_file);
	fseek(basic_program_file, 0, SEEK_SET);

	program_buffer_size = file_size + 1; // 1 byte for null terminator
	*program_buffer = (char *)calloc(program_buffer_size, sizeof(char));

	if (*program_buffer == NULL)
	{
		fprintf(stderr, "Failed to allocate %zd bytes of memory to read BASIC program file.", program_buffer_size);
		_exit(1);
	}

#ifdef __linux__
	fread(*program_buffer, sizeof(char), file_size, basic_program_file);
#elif defined(_MSC_VER)
	fread_s(*program_buffer, program_buffer_size, sizeof(char), file_size, basic_program_file);
#endif
}

char *basic_prompt_read_line(char *line_buffer, const size_t line_size)
{
	fputs(">>> ", stdout);
	return fgets(line_buffer, line_size, stdin);
}

void basic_interactive_shell()
{
	char line_buffer[1024];

	BASICProgram *basic_program = basic_create_program();
	BASICRuntime *runtime = basic_create_runtime(basic_program);

	if (runtime == NULL)
		return;

	puts("BASIC Interactive shell");
	puts("(Press Ctrl+C to terminate)");

	while (basic_prompt_read_line(line_buffer, 1024))
	{
		basic_clear_program(basic_program);
		basic_program->program_source = line_buffer;
		runtime->halt = 0;

		if (basic_tokenize(basic_program) != 0)
		{
			continue;
		}

		if (basic_parse_to_ast(basic_program))
		{
			continue;
		}

		// ast_display(basic_program->program_sequence);

		ASTNodeData result = basic_execute(runtime, basic_program->program_sequence);

		if (result.token_type != DTYPE_NONE)
		{
			StringLiteral result_buffer;
			ast_data_as_string(result, result_buffer);
			puts(result_buffer);
		}
	}
}

int main(int argc, char *argv[])
{
	set_log_mask(LOGMASK_ALL & ~(LOGTYPE_DEBUG));

	if (argc < 2)
	{
		basic_interactive_shell();
		return 0;
	}

	FILE *basic_program_file = fopen(argv[1], "rb");
	if (basic_program_file == NULL)
	{
		fputs("Failed to open the given file.", stderr);
		return 1;
	}

	char *program_buffer = NULL;

	read_basic_program(basic_program_file, &program_buffer);
	if (program_buffer == NULL)
	{
		fputs("Failed to read the given file.", stderr);
		return 1;
	}

	fclose(basic_program_file);

	BASICProgram *basic_program = basic_create_program();
	basic_program->program_source = program_buffer;

	interpret_basic_program(basic_program);
	basic_destroy_program(basic_program);

	free(program_buffer);
}
