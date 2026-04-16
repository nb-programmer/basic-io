#pragma once

/* AST parser (tokenizer) */

typedef enum
{
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

typedef struct
{
	StringLiteral token;
	BASICTokenType token_type;
	char *token_at;
} BASICToken;

// Structure with all the stuff needed to convert a BASIC program to an AST
typedef struct
{
	// List of tokens
	BASICToken *tokens;
	int tokens_length;
	// Stack for knowning current scope
} BASICParseTree;

const char *_cvt_whitespace_to_escape_code(char character);
