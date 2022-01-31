
#include "basic.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* BASIC TOKENIZER / LEXER */

//Known patterns to translate to a token list

//Keywords
//Make sure to update the KEYWORD_IDX_* entry in basic.h
char *PARSE_KEYWORDS[] = {"WHILE", "IF", "THEN", "ELSE", "END", "GOTO", NULL};
int PARSE_KW_COUNT = 6;

//Booleans: 0, 1
char *PARSE_BOOLEAN[] = {"FALSE", "TRUE"};

//Operators
char PARSE_OPERATORS[] = {'=', '+', '-', '*', '/', '<', '>', '!', '%'};

//Separator
char PARSE_SEPARATOR[] = {'(', ')', ','};

//Whitespace
//<newline> is a separator of program statements
char PARSE_WS_CHAR[] = {' ', '\t', '\n'};

//Function to insert new token into the token array
void basic_insert_token(BASICParseTree *ptree, BASICToken tok) {
    int new_token_size = ptree->tokens_length + 1;
    if (ptree->tokens == NULL) {
        //In case of garbage value
        ptree->tokens_length = 0;
        new_token_size = 1;

        ptree->tokens = (BASICToken *)malloc(sizeof(BASICToken) * new_token_size);
    } else
        ptree->tokens = (BASICToken *)realloc(ptree->tokens, sizeof(BASICToken) * new_token_size);

    if (ptree->tokens == NULL) {
        lprintf("LEXER", LOGTYPE_DEBUG, "Memory allocation failed");
        return;
    }

    memcpy(&(ptree->tokens[ptree->tokens_length]), &tok, sizeof(BASICToken));
    ptree->tokens_length = new_token_size;
}

//Returns 1 if given symbol is present in the given list of symbols
int token_char_contains(char *list, int list_size, char symbol) {
    for (int i=0;i<list_size;i++)
        if (symbol == list[i])
            return 1;
    return 0;
}

//Returns 1 if the given token is a keyword
int token_is_kw(char *token) {
    //Check for each keyword for match
    for (int i=0; i<PARSE_KW_COUNT; i++) {
        if (PARSE_KEYWORDS[i] == NULL) break;
        if (strcasecmp(PARSE_KEYWORDS[i], token) == 0)
            return 1;
    }
    return 0;
} 

//Returns 1 if the given token is a boolean (True / False)
int token_is_bool(char *token) {
    return (strcasecmp(PARSE_BOOLEAN[0], token) == 0 || strcasecmp(PARSE_BOOLEAN[1], token) == 0);
}

//Gives line number and character position in the given program string
void program_whereis(char *program, char *current_position, char *buffer) {

}

//Converts words/symbols to tokens. Also called "Lexer"
int basic_tokenize(BASICProgram *program) {
    BASICParseTree *ptree = &(program->program_tokens);
    char *tok_ptr, *tok_start;
    StringLiteral buffer = { 0 };
    int tok_len = 0;

    for (tok_ptr = program->program_source; *tok_ptr != '\0'; tok_ptr++) {
        //Check for digit
        if (isdigit(*tok_ptr)) {
            BASICToken tk_num;
            //Beginning of a number
            tok_start = tok_ptr;
            //Check if number is negative
            if (*(tok_start-1) == '-') tok_start--;
            //Keep searching till we run out of digits
            while ((isdigit(*tok_ptr) || *tok_ptr == '.') && *tok_ptr != '\0')
                tok_ptr++;
            //Copy the digits to a buffer
            int num_size = MIN(tok_ptr - tok_start, sizeof(StringLiteral) - 1);
            strncpy(tk_num.token, tok_start, num_size);
            tk_num.token[num_size] = '\0';
            tk_num.token_type = TOKEN_NUM;
            tk_num.token_at = tok_start;
            //Go back one symbol as the above loop moved past the current token
            tok_ptr--;
            basic_insert_token(ptree, tk_num);
            lprintf("LEXER", LOGTYPE_DEBUG, "Found number %s\n", tk_num.token);
            continue;
        }

        //Check for operators
        if (token_char_contains(PARSE_OPERATORS, sizeof(PARSE_OPERATORS), *tok_ptr)) {
            //Skip if this is a negative number sign
            if (*tok_ptr == '-' && isdigit(*(tok_ptr+1))) continue;
            BASICToken tk_op;
            tk_op.token[0] = *tok_ptr; tk_op.token[1] = '\0';
            tk_op.token_type = TOKEN_OPERATOR;
            tk_op.token_at = tok_ptr;
            basic_insert_token(ptree, tk_op);
            lprintf("LEXER", LOGTYPE_DEBUG, "Found operator %c\n", *tok_ptr);
            continue;
        }

        //Check for whitespace
        if (token_char_contains(PARSE_WS_CHAR, sizeof(PARSE_WS_CHAR), *tok_ptr)) {
            BASICToken tk_ws;
            tk_ws.token_type = TOKEN_WHITESPACE;
            tk_ws.token[0] = *tok_ptr; tk_ws.token[1] = '\0';
            tk_ws.token_at = tok_ptr;
            basic_insert_token(ptree, tk_ws);
            lprintf("LEXER", LOGTYPE_DEBUG, "Found whitespace\n");
            continue;
        } else {
            //Identifier (letters and underscore, numbers afterwards) or keyword
            if (isalpha(*tok_ptr) || *tok_ptr == '_') {
                BASICToken tk_identifier;
                tok_start = tok_ptr;
                while ((isalpha(*tok_ptr) || *tok_ptr == '_' || isdigit(*tok_ptr)) && *tok_ptr != '\0')
                    tok_ptr++;
                //Copy the name to a buffer
                int id_size = MIN(tok_ptr - tok_start, sizeof(StringLiteral) - 1);
                strncpy(tk_identifier.token, tok_start, id_size);
                tk_identifier.token[id_size] = '\0';
                tk_identifier.token_at = tok_start;
                tok_ptr--;

                //Check if what we found is a keyword
                if (token_is_kw(tk_identifier.token)) {
                    //It is a keyword
                    tk_identifier.token_type = TOKEN_KEYWORD;
                    basic_insert_token(ptree, tk_identifier);
                    lprintf("LEXER", LOGTYPE_DEBUG, "Found keyword \"%s\"\n", tk_identifier.token);
                } else if (token_is_bool(tk_identifier.token)) {
                    //It is a boolean
                    tk_identifier.token_type = TOKEN_BOOL;
                    basic_insert_token(ptree, tk_identifier);
                    lprintf("LEXER", LOGTYPE_DEBUG, "Found boolean %s\n", tk_identifier.token);
                } else {
                    //It is an identifer
                    tk_identifier.token_type = TOKEN_IDENTIFIER;
                    basic_insert_token(ptree, tk_identifier);
                    lprintf("LEXER", LOGTYPE_DEBUG, "Found identifier \"%s\"\n", tk_identifier.token);
                }

                continue;
            }

            //Check for string literal
            if (*tok_ptr == '"') {
                BASICToken tk_str;
                //Start string the next character from double-quote
                tok_start = ++tok_ptr;
                while (*tok_ptr != '"' && *tok_ptr != '\0')
                    tok_ptr++;

                if (*tok_ptr == '\0') {
                    lprintf("LEXER", LOGTYPE_ERROR, "Error! String literal is not terminated!\n");
                    return 1;
                }

                //Copy the string to a buffer
                int id_size = MIN(tok_ptr - tok_start, sizeof(StringLiteral) - 1);
                strncpy(tk_str.token, tok_start, id_size);
                tk_str.token[id_size] = '\0';
                tk_str.token_type = TOKEN_STRING;
                tk_str.token_at = tok_start;
                basic_insert_token(ptree, tk_str);
                lprintf("LEXER", LOGTYPE_DEBUG, "Found string literal \"%s\"\n", tk_str.token);
                continue;
            }

            //Check for separator
            if (token_char_contains(PARSE_SEPARATOR, sizeof(PARSE_SEPARATOR), *tok_ptr)) {
                BASICToken tk_sp;
                tk_sp.token[0] = *tok_ptr; tk_sp.token[1] = '\0';
                tk_sp.token_type = TOKEN_SEPARATOR;
                tk_sp.token_at = tok_ptr;
                basic_insert_token(ptree, tk_sp);
                lprintf("LEXER", LOGTYPE_DEBUG, "Found separator %c\n", *tok_ptr);
                continue;
            }
        }
    }

    BASICToken tk_end;
    tk_end.token_type = TOKEN_END;
    tk_end.token[0] = '\0';
    tk_end.token_at = tok_ptr;
    basic_insert_token(ptree, tk_end);
    lprintf("LEXER", LOGTYPE_DEBUG, "End of program\n");

    lprintf("LEXER", LOGTYPE_DEBUG, "Finished tokenization of program\n");

    return 0;
}
