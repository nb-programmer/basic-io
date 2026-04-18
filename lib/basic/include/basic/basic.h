#pragma once

/* BASIC Program parser / interpreter common header file */

/**
 * BASIC interpreter steps:
 *
 * 0. BASIC program source code input
 * 1. BASIC Lexer (Program source to tokens)
 * 2. BASIC Parser (Tokens to AST)
 * 3. Execute AST sequentially (Program run)
 *
 */

#include "ast.h"
#include "basic_program.h"
#include "basic_token.h"
#include "basic_lexer.h"
#include "basic_parser.h"
#include "basic_runner.h"
