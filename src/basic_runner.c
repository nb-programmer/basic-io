
#include "basic.h"
#include "ast.h"
#include "stack.h"
#include "utils.h"

//Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//This code will directly interpret from the abstract syntax tree

BASICVariable *basic_find_variable(BASICRuntime *runtime, char var_name[]) {
    if (runtime->variables == NULL) return NULL;
    for (int i=0;i<runtime->var_count;i++)
        if (strcmp(runtime->variables[i].name, var_name) == 0)
            return &(runtime->variables[i]);
    return NULL;
}

ASTNodeData basic_get_variable(BASICRuntime *runtime, char var_name[]) {
    BASICVariable *var;
    if ((var = basic_find_variable(runtime, var_name)) == NULL) {
        lprintf("EXEC", LOGTYPE_ERROR, "Error: Tried to read an undefined variable %s\n", var_name);
        runtime->halt = 1;
        return ASTVOID;
    }
    return var->value;
}

void basic_set_variable(BASICRuntime *runtime, char var_name[], ASTNodeData value) {
    BASICVariable *var;
    int new_size;
    if ((var = basic_find_variable(runtime, var_name)) == NULL) {
        if (runtime->variables == NULL && runtime->var_count == 0) {
            new_size = 1;
            runtime->variables = (BASICVariable *)malloc(sizeof(BASICVariable));
            if (runtime->variables == NULL) {
                lprintf("EXEC", LOGTYPE_ERROR, "Error: Failed to allocate memory for variables\n");
                runtime->halt = 1;
                return;
            }
        } else {
            new_size = runtime->var_count + 1;
            runtime->variables = (BASICVariable *)realloc(runtime->variables, sizeof(BASICVariable) * new_size);
            if (runtime->variables == NULL) {
                lprintf("EXEC", LOGTYPE_ERROR, "Error: Failed to allocate memory for variables\n");
                runtime->halt = 1;
                return;
            }
        }

        var = &(runtime->variables[runtime->var_count]);
        strcpy(var->name, var_name);
        runtime->var_count = new_size;
    }
    memcpy(&(var->value), &value, sizeof(ASTNodeData));
}

//Return a function pointer to a BASIC function if it exists
basic_function basic_decode_function(char fn_name[]) {
    if (strcasecmp(fn_name, "print") == 0)
        return _basic_fn_print;
    if (strcasecmp(fn_name, "max") == 0)
        return _basic_fn_max;
    if (strcasecmp(fn_name, "min") == 0)
        return _basic_fn_min;
    if (strcasecmp(fn_name, "sleep") == 0)
        return _basic_fn_sleep;
    if (strcasecmp(fn_name, "int") == 0)
        return _basic_fn_toint;
    if (strcasecmp(fn_name, "float") == 0)
        return _basic_fn_toflt;
    if (strcasecmp(fn_name, "random") == 0)
        return _basic_fn_rand;
    if (strcasecmp(fn_name, "irandom") == 0)
        return _basic_fn_irand;

    return (basic_function)0;
}

ASTNodeData basic_evaluate_operation(BASICRuntime *runtime, ASTNode *expression) {
    ASTOperator op = expression->data.token.op;
    ASTOperatorType opt = ast_get_operator_type(op);
    ASTNode *operand_ptr = expression->child;
    ASTNodeData operands[3], result = ASTVOID;

    switch (opt) {
    case OPTYPE_UNARY:
        operands[0] = basic_evaluate_node(runtime, operand_ptr);
        break;
    case OPTYPE_BINARY:
        if (op == OP_ASSIGN) {
            //Variable assignment is done directly
            basic_var_assignment(runtime, operand_ptr);
        } else {
            operands[0] = basic_evaluate_node(runtime, operand_ptr);
            if (operand_ptr->next == NULL) {
                lprintf("EXEC", LOGTYPE_ERROR, "Error: Binary operator is given only 1 operand\n");
                return ASTVOID;
            }
            operands[1] = basic_evaluate_node(runtime, operand_ptr->next);
            int error = ast_evaluate_binary(op, operands[0], operands[1], &result);
            if (error != 0) {
                runtime->halt = 1;
                lprintf("EXEC", LOGTYPE_ERROR, "Error occured evaluating an expression: ");
                switch (error) {
                case 1:
                    printf("Incompatible datatypes for operands"); break;
                case 2:
                    printf("Incorrect operand used for binary operation"); break;
                case 3:
                    printf("Division by zero"); break;
                default:
                    printf("Unknown error occured");
                }
                printf("\n");
            }
            break;
        }
    }

    return result;
}

/* Public functions */

ASTNodeData basic_evaluate_node(BASICRuntime *runtime, ASTNode *node) {
    if (runtime->halt) return ASTVOID;

    switch (node->type) {
    case AST_IMMEDIATE:
        return node->data;
    case AST_VARIABLE:
        return basic_get_variable(runtime, node->data.token.variable_name);
    case AST_FUNC_CALL:
    {
        basic_function fn_call = basic_decode_function(node->data.token.kw);
        if (fn_call == NULL) {
            lprintf("EXEC", LOGTYPE_DEBUG, "Unknown function %s tried to be called\n", node->data.token.kw);
            return ASTVOID;
        }
        return fn_call(runtime, node->child);
    }
    case AST_OPERATION:
        return basic_evaluate_operation(runtime, node);
    case AST_EXPRESSION:
        //Expression within an expression requires another node to join them
        return basic_evaluate_node(runtime, node->child);
    case AST_CONDITION:
        return basic_evaluate_node(runtime, node->child);
    }

    return ASTVOID;
}

void basic_var_assignment(BASICRuntime *runtime, ASTNode *args) {
    ASTNode *var_to_assign = args;
    ASTNode *val_to_assign = args->next;
    if (var_to_assign->type != AST_VARIABLE) {
        lprintf("EXEC", LOGTYPE_DEBUG, "Trying to assign expression to non-variable token\n");
        runtime->halt = 1;
        return;
    }
    char *var_name = var_to_assign->data.token.variable_name;
    if (val_to_assign == NULL) {
        lprintf("EXEC", LOGTYPE_DEBUG, "Trying to assign NULL to the variable %s\n", var_name);
        runtime->halt = 1;
        return;
    }

    //Assign variable the evaluated result, LHS <- RHS
    basic_set_variable(runtime, var_name, basic_evaluate_node(runtime, val_to_assign));
}

//Executes a BASICProgram object (inside the runtime)
ASTNodeData basic_execute(BASICRuntime *runtime, ASTNode *pc) {
    //'pc' is our "program counter"
    //'runtime' stores all variables and their values, and such data for running the program

    //Executes the current sequence of instructions till end of list
    while (!runtime->halt) {
        ASTNode *current_pc = pc;

        if (current_pc == NULL) {
            StackNode *nxt_pc_stk = stack_pop(&(runtime->traverse_stack));
            if (nxt_pc_stk == NULL)
                break;
            else {
                pc = (ASTNode *) nxt_pc_stk->data;
                stack_delete_node(nxt_pc_stk);
                continue;
            }
        } else {
            //Increment Program counter to next instruction
            pc = pc->next;
        }

        switch (current_pc->type) {
        case AST_PROGRAM_SEQUENCE:
            //Beginning of the program. Run from first instruction and return
            pc = current_pc->child;
            break;
        //Function call which is not expecting a return value
        case AST_FUNC_CALL:
        //Expression directly given as a statement (eg. Variable assignment)
        case AST_EXPRESSION:
        case AST_OPERATION:
            basic_evaluate_node(runtime, current_pc);
            break;
        case AST_KEYWORD:
        {
            //If needed, where to jump to, or where to run sub-routine block
            ASTNode *next_pc = NULL;
            KeywordAction kw_action = basic_evaluate_keyword_block(runtime, current_pc, &next_pc);
            switch (kw_action) {
            case KW_JMP_ONLY:
                pc = next_pc;
                break;
            case KW_JMP_AND_RET_NEXT:
            {
                StackNode *ret_node = stack_create_node();
                ret_node->data = (ASTNode *) pc;
                stack_push(&(runtime->traverse_stack), ret_node);
                pc = next_pc;
                break;
            }
            case KW_JMP_AND_RET_CURR:
            {
                StackNode *ret_node = stack_create_node();
                ret_node->data = (ASTNode *) current_pc;
                stack_push(&(runtime->traverse_stack), ret_node);
                pc = next_pc;
                break;
            }
            case KW_DO_NOTHING:
            default: break;
            }
            break;
        }
        case AST_NONE:
        default:
            break;
        }
    }

    return ASTVOID;
}

//Creates a runtime environment for running the basic interpreter
BASICRuntime *basic_create_runtime(BASICProgram *program) {
    BASICRuntime *runtime;
    runtime = (BASICRuntime *)malloc(sizeof(BASICRuntime));
    if (runtime == NULL) {
        fprintf(stderr, "Failed to create a runtime environment\n");
        return NULL;
    }
    runtime->program = program;
    runtime->halt = 0;
    runtime->var_count = 0;
    runtime->variables = NULL;
    runtime->traverse_stack = NULL;

    basic_init_constants(runtime);

    return runtime;
}

void basic_free_runtime(BASICRuntime *runtime) {
    if (runtime != NULL) {
        if (runtime->variables != NULL)
            free(runtime->variables);
        free(runtime);
    }
}

/* Private functions */

void basic_init_constants(BASICRuntime *runtime) {
    ASTNodeData pi_value; pi_value.token_type = DTYPE_FLT; pi_value.token.literal.flt = M_PI;
    ASTNodeData rand_max; rand_max.token_type = DTYPE_NUM; rand_max.token.literal.num = RAND_MAX;
    basic_set_variable(runtime, "PI", pi_value);
    basic_set_variable(runtime, "RANDOM_MAX", rand_max);
}

//Simple print function
ASTNodeData _basic_fn_print(BASICRuntime *runtime, ASTNode *args) {
    ASTNode *arg = args;
    char temp[512] = { 0 };
    while (arg != NULL) {
        ASTNodeData value = basic_evaluate_node(runtime, arg);
        ast_data_as_string(value, temp);
        printf("%s", temp);
        arg = arg->next;
        //Space separated arguments
        if (arg != NULL) printf(" ");
    }
    printf("\n");
    fflush(stdout);

    //No return value
    return ASTVOID;
}

//Function to find maximum value from given parameters
ASTNodeData _basic_fn_max(BASICRuntime *runtime, ASTNode *args) {
    ASTNode *arg = args;
    ASTNodeData ret_val = ASTVOID;
    while (arg != NULL) {
        ASTNodeData value = basic_evaluate_node(runtime, arg);
        ast_get_greater(ret_val, value, &ret_val);
        arg = arg->next;
    }
    return ret_val;
}

//Function to find minimum value from given parameters
ASTNodeData _basic_fn_min(BASICRuntime *runtime, ASTNode *args) {
    ASTNode *arg = args;
    ASTNodeData ret_val = ASTVOID;
    while (arg != NULL) {
        ASTNodeData value = basic_evaluate_node(runtime, arg);
        ast_get_lesser(ret_val, value, &ret_val);
        arg = arg->next;
    }

    return ret_val;
}

//Sleep for given number of seconds (can be fraction)
ASTNodeData _basic_fn_sleep(BASICRuntime *runtime, ASTNode *arg) {
    //Exit if no argument passed
    if (arg == NULL) return ASTVOID;
    //Evaluate the expression in the given argument
    ASTNodeData value = basic_evaluate_node(runtime, arg);
    //Convert to float
    float sleep_seconds = ast_data_to_flt(value);
    //Calculate the duration in terms microseconds (x 10^6)
    unsigned long long duration = (unsigned long long)(sleep_seconds * 1e6);
    system_usleep(duration);
    return (ASTNodeData) {0, DTYPE_NUM};
}

//Convert given integer / float to integer
ASTNodeData _basic_fn_toint(BASICRuntime *runtime, ASTNode *arg) {
    if (arg == NULL) {
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

//Convert given integer / float to float
ASTNodeData _basic_fn_toflt(BASICRuntime *runtime, ASTNode *arg) {
    if (arg == NULL) {
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

ASTNodeData _basic_fn_rand(BASICRuntime *runtime, ASTNode *arg) {
    if (arg != NULL) {
        lprintf("EXEC", LOGTYPE_ERROR, "Error: Function call does not expect any arguments\n");
        runtime->halt = 1;
        return ASTVOID;
    }
    ASTNodeData random;
    random.token.literal.flt = system_random_float();
    random.token_type = DTYPE_FLT;
    return random;
}

ASTNodeData _basic_fn_irand(BASICRuntime *runtime, ASTNode *arg) {
    if (arg != NULL) {
        lprintf("EXEC", LOGTYPE_ERROR, "Error: Function call does not expect any arguments\n");
        runtime->halt = 1;
        return ASTVOID;
    }
    ASTNodeData random;
    random.token.literal.num = system_random_int();
    random.token_type = DTYPE_NUM;
    return random;
}

/* Keyword evaluation */

KeywordAction basic_evaluate_keyword_block(BASICRuntime *runtime, ASTNode *pc, ASTNode **nextpc) {
    if (strcasecmp(pc->data.token.kw, PARSE_KEYWORDS[KEYWORD_IDX_IF]) == 0)
        return basic_eval_kw_if(runtime, pc, nextpc);
    if (strcasecmp(pc->data.token.kw, PARSE_KEYWORDS[KEYWORD_IDX_WHILE]) == 0)
        return basic_eval_kw_while(runtime, pc, nextpc);
    lprintf("EXEC", LOGTYPE_ERROR, "Found unknown keyword \"%s\"\n", pc->data.token.kw);
    runtime->halt = 1;
    return KW_DO_NOTHING;
}

KeywordAction basic_eval_kw_if(BASICRuntime *runtime, ASTNode *pc, ASTNode **nextpc) {
    /*
        If clause contains the "condition" node, a node to execute if the condition is TRUE,
        and another node if the condition is false (may be empty)
             _______IF_______
            |       |        |
        Condition   |     Program
           ...      |     Sequence
                    |     /  |  \
                Program    ......
                Sequence
                /  |  \
                 ......
    */

    ASTNode *cond_node = pc->child;
  
    //Checking stuff... so boring
    if (cond_node == NULL) {
        lprintf("EXEC", LOGTYPE_ERROR, "No condition given to the IF block\n");
        runtime->halt = 1;
        return KW_DO_NOTHING;
    }
    if (cond_node->type != AST_CONDITION) {
        lprintf("EXEC-BUG", LOGTYPE_ERROR, "Condition was expected, but somehow node type id %d found.\n", cond_node->type);
        runtime->halt = 1;
        return KW_DO_NOTHING;
    }
    ASTNode *true_path = cond_node->next;
    if (true_path == NULL) {
        lprintf("EXEC-BUG", LOGTYPE_ERROR, "No program sequence found for \"TRUE\" block\n");
        runtime->halt = 1;
        return KW_DO_NOTHING;
    }
    ASTNode *false_path = true_path->next;
    if (true_path->type != AST_PROGRAM_SEQUENCE) {
        lprintf("EXEC-BUG", LOGTYPE_ERROR, "\"TRUE\" block needs to be of type Program Sequence\n");
        runtime->halt = 1;
        return KW_DO_NOTHING;
    }
    if (false_path != NULL) {
        if (false_path->type != AST_PROGRAM_SEQUENCE) {
            lprintf("EXEC-BUG", LOGTYPE_ERROR, "\"FALSE\" block needs to be of type Program Sequence\n");
            runtime->halt = 1;
            return KW_DO_NOTHING;
        }
    }

    //Now we can do the actual evaluation
    ASTNodeData condition_evaluated = basic_evaluate_node(runtime, cond_node);
    int contition_val = ast_data_to_int(condition_evaluated);

    //Who would've guessed evaulation of IF statements can be done with an IF statement
    if (contition_val) {
        *nextpc = true_path;
        return KW_JMP_AND_RET_NEXT;
    } else if (false_path != NULL) {
        *nextpc = false_path;
        return KW_JMP_AND_RET_NEXT;
    }

    return KW_DO_NOTHING;
}

KeywordAction basic_eval_kw_while(BASICRuntime *runtime, ASTNode *pc, ASTNode **nextpc) {
    /*
        WHILE clause contains the "condition" node, and a node to execute till the condition
        becomes FALSE
             _____WHILE_____
            |               |
        Condition        Program
           ...           Sequence
                         /  |  \
                          ......
    */

    ASTNode *cond_node = pc->child;
  
    //Checking stuff... so boring
    if (cond_node == NULL) {
        lprintf("EXEC", LOGTYPE_ERROR, "No condition given to the IF block\n");
        runtime->halt = 1;
        return KW_DO_NOTHING;
    }
    if (cond_node->type != AST_CONDITION) {
        lprintf("EXEC-BUG", LOGTYPE_ERROR, "Condition was expected, but somehow node type id %d found.\n", cond_node->type);
        runtime->halt = 1;
        return KW_DO_NOTHING;
    }
    ASTNode *true_path = cond_node->next;
    if (true_path == NULL) {
        lprintf("EXEC-BUG", LOGTYPE_ERROR, "No program sequence found for \"TRUE\" block\n");
        runtime->halt = 1;
        return KW_DO_NOTHING;
    }

    //Now we can do the actual evaluation
    ASTNodeData condition_evaluated = basic_evaluate_node(runtime, cond_node);
    int contition_val = ast_data_to_int(condition_evaluated);

    //Jump to body and then return back to condition check
    if (contition_val) {
        *nextpc = true_path;
        return KW_JMP_AND_RET_CURR;
    }

    return KW_DO_NOTHING;
}
