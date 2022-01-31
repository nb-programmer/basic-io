
#include "utils.h"
#include "basic.h"

#include "stack.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* BASIC PARSER */

int basic_parse_id_is_fn_call(BASICToken *tokens, int idx, int len) {
    return (tokens[idx].token_type == TOKEN_IDENTIFIER
        && idx < len-1
        && tokens[idx+1].token_type == TOKEN_SEPARATOR
        && tokens[idx+1].token[0] == '('
    );
}

//Skips whitespace and reaches the next token 
void basic_token_seek_immediate(BASICParseTree *ptree, int *ptr, int end) {
    while (*ptr < end) {
        if (ptree->tokens[*ptr].token_type != TOKEN_WHITESPACE) break;
        (*ptr)++;
    }
}

int basic_parse_to_ast_between(BASICParseTree *ptree, ASTNode *root, int from, int to) {
    return basic_parse_to_ast_between_level(ptree, root, from, to, 0, 1, NULL);
}

int basic_parse_to_ast_between_onlyexpr(BASICParseTree *ptree, ASTNode *root, int from, int to) {
    return basic_parse_to_ast_between_level(ptree, root, from, to, 0, 0, NULL);
}

int basic_parse_to_ast_between_level(BASICParseTree *ptree, ASTNode *root, int from, int to, int level, int allow_keyword, int *next_ptr) {
    int cb_ret;
    for (int i = from; i < to; i++) {
        switch (ptree->tokens[i].token_type) {
        case TOKEN_IDENTIFIER:
        {
            //The identifier may be a function call
            if (basic_parse_id_is_fn_call(ptree->tokens, i, to)) {
                cb_ret = basic_parse_form_function(ptree, root, i, to, &i);
                if (cb_ret != 0) return cb_ret;
                break;
            }
        }

        //Not a function call, so it may be an expression
        case TOKEN_NUM:
        case TOKEN_STRING:
        case TOKEN_SEPARATOR:
        case TOKEN_BOOL:
        case TOKEN_OPERATOR:
            cb_ret = basic_parse_form_expression(ptree, root, i, to, &i);
            if (cb_ret != 0) return cb_ret;
            break;

        //Keywords
        case TOKEN_KEYWORD:
        {
            lprintf("AST", LOGTYPE_DEBUG, "Parse keyword \"%s\"\n", ptree->tokens[i].token);

            if (strcasecmp(ptree->tokens[i].token, PARSE_KEYWORDS[KEYWORD_IDX_IF]) == 0) {
                //"IF" clause. Has an expression and a program sequence to execute if the value of the expression is non-zero (true)
                ASTNode *if_node = ast_create_node();
                ASTNode *condition_node = ast_create_node();
                ASTNode *if_true_node = ast_create_node();
                ASTNode *if_false_node = ast_create_node();
                if_node->type = AST_KEYWORD;
                if_node->data.token_type = DTYPE_SYMB;
                strcpy(if_node->data.token.kw, PARSE_KEYWORDS[KEYWORD_IDX_IF]);
                condition_node->type = AST_CONDITION;
                condition_node->data = ASTVOID;
                if_true_node->type = AST_PROGRAM_SEQUENCE;
                if_true_node->data = ASTVOID;
                if_false_node->type = AST_PROGRAM_SEQUENCE;
                if_false_node->data = ASTVOID;

                ast_append_child(root, if_node);
                ast_append_child(if_node, condition_node);
                ast_append_child(if_node, if_true_node);
                ast_append_child(if_node, if_false_node);

                //The next part after "IF" is the condition (expression)
                i++;

                lprintf("AST", LOGTYPE_DEBUG, "Parse IF condition expression\n");
                cb_ret = basic_parse_form_expression(ptree, condition_node, i, to, &i);
                if (cb_ret != 0) return cb_ret;
                basic_token_seek_immediate(ptree, &i, to);

                //Check if there is a "THEN"
                if (ptree->tokens[i].token_type == TOKEN_KEYWORD && strcasecmp(ptree->tokens[i].token, PARSE_KEYWORDS[KEYWORD_IDX_THEN]) == 0) {
                    //Next part after "THEN" is the body, till "END" is found
                    i++;
                    int next_level = level + 1;
                    lprintf("AST", LOGTYPE_DEBUG, "Parse program statements at scope level %d\n", next_level);
                    int next_pos = -1;
                    int ret = basic_parse_to_ast_between_level(ptree, if_true_node, i, to, next_level, 1, &next_pos);
                    if (ret >= 0 && next_pos >= 0) {
                        //Set token position to the next instruction returned
                        i = next_pos;
                        if (ret == 2) {
                            //Else route exists. Go to next symbol to find the program sequence within the else clause body
                            i++;
                            lprintf("AST", LOGTYPE_DEBUG, "Parse program statements for %s at scope level %d\n", PARSE_KEYWORDS[KEYWORD_IDX_ELSE], next_level);
                            ret = basic_parse_to_ast_between_level(ptree, if_false_node, i, to, next_level, 1, &next_pos);
                            //We basically do the same check again, for one last time
                            if (ret >= 0 && next_pos >= 0) {
                                i = next_pos;
                                if (ret == 2) {
                                    //This time, if another else is found at the same level, throw an error
                                    lprintf("AST", LOGTYPE_ERROR, "Error: %s clause cannot have more than one %s statements.\n", PARSE_KEYWORDS[KEYWORD_IDX_IF], PARSE_KEYWORDS[KEYWORD_IDX_ELSE]);
                                    return -3;
                                }
                            }
                        }
                    } else {
                        lprintf("AST", LOGTYPE_ERROR, "An error (code %d) occured while parsing IF clause\n", ret);
                        return ret;
                    }
                } else {
                    lprintf("AST", LOGTYPE_ERROR, "Error: Expected a \"%s\" keyword after specifying expression\n", PARSE_KEYWORDS[KEYWORD_IDX_THEN]);
                    return -2;
                }
            } else if (strcasecmp(ptree->tokens[i].token, PARSE_KEYWORDS[KEYWORD_IDX_WHILE]) == 0) {
                //"WHILE" clause. Has an expression and a program sequence to execute till the value of the expression becomes zero (false)
                ASTNode *while_node = ast_create_node();
                ASTNode *condition_node = ast_create_node();
                ASTNode *while_true_node = ast_create_node();
                while_node->type = AST_KEYWORD;
                while_node->data.token_type = DTYPE_SYMB;
                strcpy(while_node->data.token.kw, PARSE_KEYWORDS[KEYWORD_IDX_WHILE]);
                condition_node->type = AST_CONDITION;
                condition_node->data = ASTVOID;
                while_true_node->type = AST_PROGRAM_SEQUENCE;
                while_true_node->data = ASTVOID;

                ast_append_child(root, while_node);
                ast_append_child(while_node, condition_node);
                ast_append_child(while_node, while_true_node);

                //The next part after "WHILE" is the condition (expression)
                i++;

                lprintf("AST", LOGTYPE_DEBUG, "Parse WHILE condition expression\n");
                cb_ret = basic_parse_form_expression(ptree, condition_node, i, to, &i);
                if (cb_ret != 0) return cb_ret;
                basic_token_seek_immediate(ptree, &i, to);

                //Check if there is a "THEN"
                if (ptree->tokens[i].token_type == TOKEN_KEYWORD && strcasecmp(ptree->tokens[i].token, PARSE_KEYWORDS[KEYWORD_IDX_THEN]) == 0) {
                    //Next part after "THEN" is the body, till "END" is found
                    i++;
                    int next_level = level + 1;
                    lprintf("AST", LOGTYPE_DEBUG, "Parse program statements at scope level %d\n", next_level);
                    int next_pos = -1;
                    int ret = basic_parse_to_ast_between_level(ptree, while_true_node, i, to, next_level, 1, &next_pos);
                    if (ret >= 0 && next_pos >= 0) {
                        //Set token position to the next instruction returned
                        i = next_pos;
                        //Make sure the While statement doesn't have an Else part
                        if (ret == 2) {
                            lprintf("AST", LOGTYPE_ERROR, "Error: %s clause cannot have an %s statement.\n", PARSE_KEYWORDS[KEYWORD_IDX_WHILE], PARSE_KEYWORDS[KEYWORD_IDX_ELSE]);
                            return -3;
                        }
                    } else {
                        lprintf("AST", LOGTYPE_ERROR, "An error (code %d) occured while parsing %s clause\n", ret, PARSE_KEYWORDS[KEYWORD_IDX_WHILE]);
                        return ret;
                    }
                } else {
                    lprintf("AST", LOGTYPE_ERROR, "Error: Expected a \"%s\" keyword after specifying expression\n", PARSE_KEYWORDS[KEYWORD_IDX_THEN]);
                    return -2;
                }
            } else if (strcasecmp(ptree->tokens[i].token, PARSE_KEYWORDS[KEYWORD_IDX_ELSE]) == 0) {
                //Else clause for a matching IF clause
                if (level > 0) {
                    lprintf("AST", LOGTYPE_DEBUG, "%s statement encountered, previouse scope level is %d\n", PARSE_KEYWORDS[KEYWORD_IDX_ELSE], level);
                    //Where to resume token parsing
                    *next_ptr = i;
                    //Return 2 to indicate there is an alternate path
                    return 2;
                } else {
                    lprintf("AST", LOGTYPE_ERROR, "Error: Found an unexpected \"%s\" without corresponding %s clause\n", PARSE_KEYWORDS[KEYWORD_IDX_ELSE], PARSE_KEYWORDS[KEYWORD_IDX_IF]);
                    return -1;
                }
            } else if (strcasecmp(ptree->tokens[i].token, PARSE_KEYWORDS[KEYWORD_IDX_END]) == 0) {
                //"END" keyword. We are in some kind of body segment of a clause
                //Check if we are actually inside a body by checking out current level
                //If it is 0, we are at the main sequence, but an extra "END" was present
                if (level > 0) {
                    lprintf("AST", LOGTYPE_DEBUG, "End of program statements at scope level %d\n", level);
                    //Where to resume token parsing
                    *next_ptr = i;
                    //Return 1 to indicate it is the end of the current sequence
                    return 1;
                } else {
                    lprintf("AST", LOGTYPE_ERROR, "Error: Found an unexpected \"%s\" without corresponding %s/%s/%s clause\n", PARSE_KEYWORDS[KEYWORD_IDX_END], PARSE_KEYWORDS[KEYWORD_IDX_IF], PARSE_KEYWORDS[KEYWORD_IDX_ELSE], PARSE_KEYWORDS[KEYWORD_IDX_WHILE]);
                    return -1;
                }
            }
        }
            break;
        }
    }

    if (level > 0) {
        lprintf("AST", LOGTYPE_ERROR, "Error: End of program reached inside %s/%s clause, without encountering \"%s\"\n", PARSE_KEYWORDS[KEYWORD_IDX_IF], PARSE_KEYWORDS[KEYWORD_IDX_WHILE], PARSE_KEYWORDS[KEYWORD_IDX_END]);
        return -2;
    }

    return 0;
}

int basic_parse_to_ast(BASICProgram *program) {
    BASICParseTree *ptree = &(program->program_tokens);
    ASTNode *prog = program->program_sequence;
    lprintf("AST", LOGTYPE_DEBUG, "Found %d tokens in the token list\n", ptree->tokens_length);
    int ret_code = basic_parse_to_ast_between(ptree, prog, 0, ptree->tokens_length);
    if (ret_code == 0)
        lprintf("AST", LOGTYPE_DEBUG, "Finished parsing the program\n");
    else
        lprintf("AST", LOGTYPE_DEBUG, "Parser failed while processing token sequence\n");
    return ret_code;
}


void basic_parse_pushtok(StackNode **top, ASTNode *tok) {
    StackNode *opr_tok = stack_create_node();
    opr_tok->data = tok;
    stack_push(top, opr_tok);
}
ASTNode *basic_parse_poptok(StackNode **top) {
    ASTNode *p_tok = NULL;
    StackNode *opr_tok = stack_pop(top);
    if (opr_tok != NULL) {
        p_tok = (ASTNode *)(opr_tok->data);
        stack_delete_node(opr_tok);
    }
    return p_tok;
}
void basic_parse_enqueuetok(Queue *queue, ASTNode *tok) {
    QueueNode *opr_tok = queue_create_node();
    opr_tok->data = tok;
    queue_enqueue(queue, opr_tok);
}
ASTNode *basic_parse_dequeuetok(Queue *queue) {
    ASTNode *p_tok = NULL;
    QueueNode *opr_tok = queue_dequeue(queue);
    if (opr_tok != NULL) {
        p_tok = (ASTNode *)(opr_tok->data);
        queue_delete_node(opr_tok);
    }
    return p_tok;
}

int basic_expr_make_tree(Queue *infix_queue, ASTNode *root) {
    //Pull elements from the infix expression and convert it to a tree
    StackNode *operator_stack = NULL, *operand_stack = NULL;
    Queue postfix_queue; postfix_queue.front = NULL, postfix_queue.rear = NULL;
    ASTNode *nxttok, *poptok, *expr_node = ast_create_node(), *root_exp_paren1 = ast_create_node(), *root_exp_paren2 = ast_create_node();

    expr_node->type = AST_EXPRESSION;
    expr_node->data = ASTVOID;
    root_exp_paren1->type = root_exp_paren2->type = AST_OPERATION;
    root_exp_paren1->data.token_type = root_exp_paren2->data.token_type = DTYPE_SYMB;
    root_exp_paren1->data.token.op = OP_OPEN_PAREN; root_exp_paren2->data.token.op = OP_CLOSE_PAREN;

    basic_parse_pushtok(&operator_stack, root_exp_paren1);
    basic_parse_enqueuetok(infix_queue, root_exp_paren2);

    //Read the expression in sequential order
    while ((nxttok = basic_parse_dequeuetok(infix_queue)) != NULL) {
        if (nxttok->type == AST_OPERATION) {
            if (nxttok->data.token.op == OP_OPEN_PAREN) {
                //If opening bracket, put it on stack
                basic_parse_pushtok(&operator_stack, nxttok);
            } else if (nxttok->data.token.op == OP_CLOSE_PAREN) {
                //If closing bracket, pop out the operators with precedence and put in expression
                while ((poptok = basic_parse_poptok(&operator_stack)) != NULL) {
                    if (poptok->data.token.op == OP_OPEN_PAREN) break;
                    basic_parse_enqueuetok(&postfix_queue, poptok);
                }
            } else {
                //Any kind of operation will pop the stack till higher precedence
                do {
                    if (operator_stack == NULL) break;
                    poptok = (ASTNode *) operator_stack->data;
                    if (ast_operator_precedence(poptok->data.token.op) >= ast_operator_precedence(nxttok->data.token.op)) break;
                    basic_parse_enqueuetok(&postfix_queue, poptok);
                } while (basic_parse_poptok(&operator_stack) != NULL);
                basic_parse_pushtok(&operator_stack, nxttok);
            }
        } else {
            //Put all operands in postfix queue
            basic_parse_enqueuetok(&postfix_queue, nxttok);
        }
    }

    //Convert postfix expression to a tree
    while ((nxttok = basic_parse_dequeuetok(&postfix_queue)) != NULL) {
        if (nxttok->type == AST_OPERATION) {
            //Final sequence should not contain any parenthesis
            if (nxttok->data.token.op == OP_OPEN_PAREN || nxttok->data.token.op == OP_CLOSE_PAREN) {
                lprintf("AST", LOGTYPE_ERROR, "Error: Unexpectedly found unpaired parenthesis in expression\n");
                return 1;
            } else {
                ASTNode *op1, *op2;
                if (ast_get_operator_type(nxttok->data.token.op) == OPTYPE_UNARY) {
                    //Pop two values from the stack and make them the child
                    op1 = basic_parse_poptok(&operand_stack);
                    if (op1 == NULL) {
                        lprintf("AST", LOGTYPE_ERROR, "Error: Operator expected 1 operand, but none found\n");
                        return 1;
                    }

                    //Make the operand the child of this operator
                    ast_append_child(nxttok, op1);

                    //Push back this operator
                    basic_parse_pushtok(&operand_stack, nxttok);
                } else if (ast_get_operator_type(nxttok->data.token.op) == OPTYPE_BINARY) {
                    //Pop two values from the stack and make them the child
                    op1 = basic_parse_poptok(&operand_stack); op2 = basic_parse_poptok(&operand_stack);
                    if (op1 == NULL) {
                        lprintf("AST", LOGTYPE_ERROR, "Error: Operator expected 2 operands, but none found\n");
                        return 1;
                    }
                    if (op2 == NULL) {
                        lprintf("AST", LOGTYPE_ERROR, "Error: Operator expecterd 2 operands, but 1 found\n");
                        return 1;
                    }

                    //Make the operands the children of this operator
                    //Since these were in the stack, they are in opposite order
                    ast_append_child(nxttok, op2);
                    ast_append_child(nxttok, op1);

                    //Push back this operator
                    basic_parse_pushtok(&operand_stack, nxttok);
                }
            }
        } else
            basic_parse_pushtok(&operand_stack, nxttok);
    }

    poptok = basic_parse_poptok(&operand_stack);
    if (poptok == NULL) {
        lprintf("AST", LOGTYPE_ERROR, "Error evaluating the expression: Too many outputs in stack\n");
        return 1;
    }

    //Join the expression structure with the root node
    ast_append_child(expr_node, poptok);
    ast_append_child(root, expr_node);

    return 0;
}

//Convert an expression into prefix notation and create an AST
int basic_parse_form_expression(BASICParseTree *ptree, ASTNode *root, int parse_from, int parse_to, int *parse_new_pos) {
    lprintf("AST", LOGTYPE_DEBUG, "Trying to find expression between tokens %d and %d\n", parse_from, parse_to);
    int expr_start_idx = parse_from, expr_end_idx = -1, scope_level = 0;

    BASICToken *expr = ptree->tokens;
    *parse_new_pos = parse_from;

    //Expression ends when we reach end of current range, or a new line, or end of program
    while (*parse_new_pos < parse_to) {
        if (expr[*parse_new_pos].token_type == TOKEN_SEPARATOR) {
            if (expr[*parse_new_pos].token[0] == '(') scope_level++;
            else if (expr[*parse_new_pos].token[0] == ')') scope_level--;
        }
        if (expr[*parse_new_pos].token_type == TOKEN_END
            || expr[*parse_new_pos].token_type == TOKEN_KEYWORD
            || (expr[*parse_new_pos].token_type == TOKEN_WHITESPACE && expr[*parse_new_pos].token[0]=='\n' && scope_level==0)
            //|| (expr[*parse_new_pos].token_type == TOKEN_SEPARATOR && expr[*parse_new_pos].token[0]==')' && scope_level==0)
        ) break;
        (*parse_new_pos)++;
    }

    //Save expression range and go back one token
    expr_end_idx = (*parse_new_pos)--;
    lprintf("AST", LOGTYPE_DEBUG, "Expression found from token %d to %d\n", expr_start_idx, expr_end_idx);

    //Construct a stack of AST nodes to then rearrange to a tree structure
    Queue infix_queue;
    infix_queue.front = infix_queue.rear = NULL;
    for (int i = expr_start_idx; i < expr_end_idx; i++) {
        if (expr[i].token_type == TOKEN_END) break;
        ASTNode *operator_node = ast_create_node();

        //Translate tokens to appropriate nodes
        switch (expr[i].token_type) {
        case TOKEN_NUM:
            //We can either have an integer (no decimal point) or float (with decimal point)
            operator_node->type = AST_IMMEDIATE;
            if (string_is_float(expr[i].token)) {
                lprintf("AST", LOGTYPE_DEBUG, "Parse floating number\n");
                operator_node->data.token_type = DTYPE_FLT;
                operator_node->data.token.literal.flt = atof(expr[i].token);
            } else {
                lprintf("AST", LOGTYPE_DEBUG, "Parse integer number\n");
                operator_node->data.token_type = DTYPE_NUM;
                operator_node->data.token.literal.num = atoi(expr[i].token);
            }
            break;
        case TOKEN_STRING:
            lprintf("AST", LOGTYPE_DEBUG, "Parse string\n");
            operator_node->type = AST_IMMEDIATE;
            operator_node->data.token_type = DTYPE_STR;
            strcpy(operator_node->data.token.literal.str, expr[i].token);
            break;
        case TOKEN_BOOL:
            //Booleans are same as just setting value to 0 or 1
            lprintf("AST", LOGTYPE_DEBUG, "Parse boolean\n");
            operator_node->type = AST_IMMEDIATE;
            operator_node->data.token_type = DTYPE_NUM;
            if (strcasecmp(expr[i].token, PARSE_BOOLEAN[0]) == 0)
                operator_node->data.token.literal.num = 0;
            else if (strcasecmp(expr[i].token, PARSE_BOOLEAN[1]) == 0)
                operator_node->data.token.literal.num = 1;
            break;
        case TOKEN_KEYWORD:
            lprintf("AST", LOGTYPE_ERROR, "Error: Unexpected keyword '%s' found in expression.\n", expr[i].token);
            return 1;

        case TOKEN_SEPARATOR:
        case TOKEN_OPERATOR:
            lprintf("AST", LOGTYPE_DEBUG, "Parse operator\n");
            operator_node->type = AST_OPERATION;
            operator_node->data.token_type = DTYPE_SYMB;
            switch (expr[i].token[0]) {
            case '+': operator_node->data.token.op = OP_ADD; break;
            case '-': operator_node->data.token.op = OP_SUB; break;
            case '*': operator_node->data.token.op = OP_MUL; break;
            case '/': operator_node->data.token.op = OP_DIV; break;
            case '%': operator_node->data.token.op = OP_MOD; break;
            case '>': operator_node->data.token.op = OP_GT; break;
            case '<': operator_node->data.token.op = OP_LT; break;
            case '!': operator_node->data.token.op = OP_NOT; break;
            case '(': operator_node->data.token.op = OP_OPEN_PAREN; break;
            case ')': operator_node->data.token.op = OP_CLOSE_PAREN; break;
            case '=':
                //If we are in a condition node
                if (root->type == AST_CONDITION)
                    //We are testing for equality
                    operator_node->data.token.op = OP_EQ;
                else
                    //We are assigning an expression to an identifier
                    operator_node->data.token.op = OP_ASSIGN;
                break;
            }
            break;
        case TOKEN_IDENTIFIER:
            if (basic_parse_id_is_fn_call(expr, i, expr_end_idx)) {
                lprintf("AST", LOGTYPE_DEBUG, "Parse function call %s\n", expr[i].token);
                operator_node->type = AST_EXPRESSION;
                operator_node->data.token_type = DTYPE_SYMB;
                operator_node->data = ASTVOID;
                basic_parse_form_function(ptree, operator_node, i, expr_end_idx, &i);
            } else {
                lprintf("AST", LOGTYPE_DEBUG, "Parse identifier %s\n", expr[i].token);
                operator_node->type = AST_VARIABLE;
                operator_node->data.token_type = DTYPE_SYMB;
                strcpy(operator_node->data.token.variable_name, expr[i].token);
            }
            break;
        default:
            operator_node->type = AST_NONE;
        }

        if (operator_node->type != AST_NONE)
            basic_parse_enqueuetok(&infix_queue, operator_node);
        else
            ast_delete_node(operator_node);
    }

    basic_expr_make_tree(&infix_queue, root);

    return 0;
}

int basic_parse_form_function(BASICParseTree *ptree, ASTNode *root, int parse_from, int parse_to, int *parse_new_pos) {
    //Function call with possibly multiple argument expressions
    int scope_level = 0, arg_start = parse_from + 2, arg_end = -1;
    BASICToken *func = ptree->tokens;

    //Create a node to call a function
    ASTNode *fn_call_node = ast_create_node();
    fn_call_node->type = AST_FUNC_CALL;
    strcpy(fn_call_node->data.token.kw, func[*parse_new_pos].token);
    ast_append_child(root, fn_call_node);

    //Display information
    lprintf("AST", LOGTYPE_DEBUG, "Function call to \"%s\"\n", func[*parse_new_pos].token);
    lprintf("AST", LOGTYPE_DEBUG, "Parsing function call argument list\n");
    //Increment to reach the '('
    (*parse_new_pos)++;
    while (*parse_new_pos < parse_to) {
        if (func[*parse_new_pos].token_type == TOKEN_SEPARATOR) {
            if (func[*parse_new_pos].token[0] == '(')
                scope_level++;
            else if (func[*parse_new_pos].token[0] == ')') {
                scope_level--;
                if (scope_level == 0) {
                    arg_end = *parse_new_pos;
                    lprintf("AST", LOGTYPE_DEBUG, "Function argument parse:\n");
                    basic_parse_to_ast_between_onlyexpr(ptree, fn_call_node, arg_start, arg_end);
                    lprintf("AST", LOGTYPE_DEBUG, "End of function call \"%s\"\n", fn_call_node->data.token.kw);
                    break;
                }
            } else if (func[*parse_new_pos].token[0] == ',') {
                if (scope_level == 1) {
                    arg_end = *parse_new_pos;
                    lprintf("AST", LOGTYPE_DEBUG, "Function argument parse:\n");
                    basic_parse_to_ast_between_onlyexpr(ptree, fn_call_node, arg_start, arg_end);
                    arg_start = (*parse_new_pos)+1;
                    lprintf("AST", LOGTYPE_DEBUG, "Function argument parse done\n");
                }
            }
        }
        (*parse_new_pos)++;
    }

    if (arg_end == -1) {
        lprintf("AST", LOGTYPE_ERROR, "Error: Function argument list expected to end, but no ending ')' found\n");
        return -1;
    }

    lprintf("AST", LOGTYPE_DEBUG, "Function argument list ends at token %d\n", arg_end);
    return 0;
}