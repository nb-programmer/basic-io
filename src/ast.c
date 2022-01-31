
#include "ast.h"
#include "utils.h"

//Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNodeData ASTVOID = {0, DTYPE_NONE};

ASTNode *ast_create_node() {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Failed to create a ASTNode structure\n");
        return NULL;
    }
    node->type = AST_NONE;
    node->next = NULL;
    node->child = NULL;
    return node;
}

//Adds a node as a child to the given parent node. If the parent has children
//then it sets it as the child's last sibling node's sibling.
void ast_append_child(ASTNode *parent, ASTNode *node) {
    if (parent == NULL || node == NULL) return;
    if (parent->child == NULL)
        parent->child = node;
    else {
        ASTNode *ptr = parent->child;
        while (ptr->next != NULL) ptr = ptr->next;
        ptr->next = node;
    }
}

void ast_delete_node(ASTNode *node) {
    if (node == NULL) return;
    free(node);
}

void ast_delete_children_cascade(ASTNode *root) {
    if (root == NULL) return;
    
    //Find child node
    ASTNode *ptr = root->child;
    if (ptr == NULL) return; 

    //Iterate through it's siblings
    while (ptr != NULL) {
        ASTNode *next = ptr->next;
        //Delete children nodes recursively
        ast_delete_children_cascade(ptr);
        //Delete current node
        ast_delete_node(ptr);
        //Go to next sibling
        ptr = next;
    }
}

void ast_display_level(ASTNode *node, int level) {
    ASTNode *ptr = node;
    while (ptr != NULL) {
        print_level_space(level); printf("Node: {\n");
        print_level_space(level+1); printf("Type: ");
        switch (ptr->type) {
        case AST_PROGRAM_SEQUENCE:
            printf("Program sequence"); break;
        case AST_FUNC_CALL:
            printf("Function %s", ptr->data.token.kw); break;
        case AST_KEYWORD:
            printf("Keyword %s", ptr->data.token.kw); break;
        case AST_CONDITION:
            printf("Condition"); break;
        case AST_VARIABLE:
            printf("Variable\n");
            print_level_space(level+1); printf("Name: %s", ptr->data.token.variable_name);
            break;
        case AST_EXPRESSION:
            printf("Expression");
            break;
        case AST_IMMEDIATE:
        {
            printf("Immediate\n");
            StringLiteral buffer = { 0 };
            ast_data_as_string(ptr->data, buffer);
            print_level_space(level+1); printf("Value: %s", buffer);
        }    
            break;
        case AST_OPERATION:
            printf("Operation\n");
            print_level_space(level+1); printf("Op type: ");
            switch (ptr->data.token.op) {
            case OP_ADD: printf("Add"); break;
            case OP_SUB: printf("Subtract"); break;
            case OP_MUL: printf("Multiply"); break;
            case OP_DIV: printf("Divide"); break;
            case OP_MOD: printf("Modulo"); break;
            case OP_LT: printf("Less-than"); break;
            case OP_GT: printf("Greater-than"); break;
            case OP_ASSIGN: printf("Assignment"); break;
            case OP_OPEN_PAREN: printf("Open parenthesis"); break;
            case OP_CLOSE_PAREN: printf("Close parenthesis"); break;
            case OP_EQ: printf("Equals"); break;
            case OP_NEGATE: printf("Negation"); break;
            case OP_NOT: printf("Not"); break;
            default: printf("Unknown");
            }
            break;
        default:
            printf("Unknown");
        }
        printf("\n");
        if (ptr->child != NULL) {
            print_level_space(level+1); printf("Child nodes: [\n");
            ast_display_level(ptr->child, level + 2);
            print_level_space(level+1); printf("]\n");
        }
        ptr = ptr->next;
        print_level_space(level);
        printf("}");
        if (ptr != NULL) printf(",");
        printf("\n");
    }
}

void ast_display(ASTNode *node) {
    ast_display_level(node, 0);
}

//Converts the given data to a string representation. Must pass an adequately large buffer 
void ast_data_as_string(ASTNodeData ast_data, char *buffer) {
    switch (ast_data.token_type) {
    case DTYPE_STR: strcpy(buffer, ast_data.token.literal.str); break;
    case DTYPE_NUM: sprintf(buffer, "%d", ast_data.token.literal.num); break;
    case DTYPE_FLT: sprintf(buffer, "%f", ast_data.token.literal.flt); break;
    case DTYPE_NONE: strcpy(buffer, "void"); break;
    }
}

int ast_data_to_int(ASTNodeData ast_data) {
    switch (ast_data.token_type) {
    case DTYPE_NUM: return ast_data.token.literal.num;
    case DTYPE_FLT: return (int)(ast_data.token.literal.flt);
    case DTYPE_STR:
    case DTYPE_NONE:
    default: return 0;
    }
}

float ast_data_to_flt(ASTNodeData ast_data) {
    switch (ast_data.token_type) {
    case DTYPE_NUM:return (float)ast_data.token.literal.num;
    case DTYPE_FLT: return ast_data.token.literal.flt;
    case DTYPE_STR:
    case DTYPE_NONE:
    default: return 0.0f;
    }
}

ASTOperatorType ast_get_operator_type(ASTOperator op) {
    switch (op) {
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
    case OP_EQ:
    case OP_LT:
    case OP_GT:
    case OP_ASSIGN:
        return OPTYPE_BINARY;
    case OP_NOT:
    case OP_NEGATE:
        return OPTYPE_UNARY;
    default:
        return OPTYPE_NONE;
    }
}

int ast_operator_precedence(ASTOperator op) {
    switch (op) {
    case OP_NEGATE:
    case OP_NOT: return 2;
    case OP_MUL:
    case OP_DIV:
    case OP_MOD: return 3;
    case OP_ADD:
    case OP_SUB: return 4;
    case OP_LT:
    case OP_GT: return 6;
    case OP_EQ: return 7;
    case OP_ASSIGN: return 14;
    }
    return 99;
}

int ast_evaluate_unary(ASTOperator op, ASTNodeData operand, ASTNodeData *result) {
    int error_code = 0;
    *result = ASTVOID;
    switch (op) {
    case OP_NOT:
        if (operand.token_type == DTYPE_NUM) {
            result->token.literal.num = !result->token.literal.num;
            result->token_type = DTYPE_NUM;
        } else error_code = 1;
        break;
    case OP_NEGATE:
        if (operand.token_type == DTYPE_NUM) {
            result->token.literal.num = -result->token.literal.num;
            result->token_type = DTYPE_NUM;
        } else if(operand.token_type == DTYPE_FLT) {
            result->token.literal.flt = -result->token.literal.flt;
            result->token_type = DTYPE_FLT;
        } else error_code = 1;
        break;
    default:
        error_code = 2;
    }

    return error_code;
}

//Calculates operation result of a binary operator
int ast_evaluate_binary(ASTOperator op, ASTNodeData a, ASTNodeData b, ASTNodeData *result) {
    int error_code = 0;
    *result = ASTVOID;

    //TODO: This could need some refactoring :)
    switch (op) {
    case OP_ADD:
    {
        //Adding different types will convert it
        switch (a.token_type) {
        case DTYPE_NUM:
            //Add a integer number to something
            if (b.token_type == DTYPE_NUM) {
                //INT + INT -> INT
                result->token.literal.num = a.token.literal.num + b.token.literal.num;
                result->token_type = DTYPE_NUM;
            } else if (b.token_type == DTYPE_FLT) {
                //INT + FLOAT -> FLOAT
                result->token.literal.flt = (float)a.token.literal.num + b.token.literal.flt;
                result->token_type = DTYPE_FLT;
            } else if (b.token_type == DTYPE_STR) {
				//INT + STR -> concatenated STR (string representation of int)
				sprintf(result->token.literal.str, "%d%s", a.token.literal.num, b.token.literal.str);
                result->token_type = DTYPE_STR;
			} else error_code = 1;
            break;
        case DTYPE_FLT:
            //Add a floating number to something
            if (b.token_type == DTYPE_FLT) {
                //FLOAT + FLOAT -> FLOAT
                result->token.literal.flt = a.token.literal.flt + b.token.literal.flt;
                result->token_type = DTYPE_FLT;
            } else if (b.token_type == DTYPE_NUM) {
                //FLOAT + INT -> FLOAT
                result->token.literal.flt = a.token.literal.flt + (float)b.token.literal.num;
                result->token_type = DTYPE_FLT;
            } else if(b.token_type == DTYPE_STR) {
				//STR + FLOAT -> concatenated STR (string representation of float)
				sprintf(result->token.literal.str, "%f%s", a.token.literal.flt, b.token.literal.str);
                result->token_type = DTYPE_STR;
			} else error_code = 1;
            break;
        case DTYPE_STR:
            if (b.token_type == DTYPE_STR) {
                //STR + STR -> concatenated STR
                strcpy(result->token.literal.str, a.token.literal.str);
                strcat(result->token.literal.str, b.token.literal.str);
                result->token_type = DTYPE_STR;
            } else if (b.token_type == DTYPE_NUM) {
                //STR + INT -> concatenated STR (string representation of int)
                sprintf(result->token.literal.str, "%s%d", a.token.literal.str, b.token.literal.num);
                result->token_type = DTYPE_STR;
            } else if (b.token_type == DTYPE_FLT) {
                //STR + FLOAT -> concatenated STR (string representation of float)
                sprintf(result->token.literal.str, "%s%f", a.token.literal.str, b.token.literal.flt);
                result->token_type = DTYPE_STR;
            } else error_code = 1;
            break;
        default:
            error_code = 1;
        }
    }
        break;
    case OP_SUB:
    {
        if (a.token_type == DTYPE_NUM) {
            if (b.token_type == DTYPE_NUM) {
                //INT - INT -> INT
                result->token.literal.num = a.token.literal.num - b.token.literal.num;
                result->token_type = DTYPE_NUM;
            } else if (b.token_type == DTYPE_FLT) {
                //INT - FLOAT -> FLOAT
                result->token.literal.flt = (float)a.token.literal.num - b.token.literal.flt;
                result->token_type = DTYPE_FLT;
            } else error_code = 1;
        } else if (a.token_type == DTYPE_FLT) {
            if (b.token_type == DTYPE_FLT) {
                //FLOAT - FLOAT -> FLOAT
                result->token.literal.flt = a.token.literal.flt - b.token.literal.flt;
                result->token_type = DTYPE_FLT;
            } else if (b.token_type == DTYPE_NUM) {
                //FLOAT - INT -> FLOAT
                result->token.literal.flt = a.token.literal.flt - (float)b.token.literal.num;
                result->token_type = DTYPE_FLT;
            } else error_code = 1;
        } else error_code = 1;
    }
        break;
    case OP_MUL:
        if (a.token_type == DTYPE_NUM) {
            if (b.token_type == DTYPE_NUM) {
                //INT * INT -> INT
                result->token.literal.num = a.token.literal.num * b.token.literal.num;
                result->token_type = DTYPE_NUM;
            } else if (b.token_type == DTYPE_FLT) {
                //INT * FLOAT -> FLOAT
                result->token.literal.flt = (float)a.token.literal.num * b.token.literal.flt;
                result->token_type = DTYPE_FLT;
            } else error_code = 1;
        } else if (a.token_type == DTYPE_FLT) {
            if (b.token_type == DTYPE_FLT) {
                //FLOAT * FLOAT -> FLOAT
                result->token.literal.flt = a.token.literal.flt * b.token.literal.flt;
                result->token_type = DTYPE_FLT;
            } else if (b.token_type == DTYPE_NUM) {
                //FLOAT * INT -> FLOAT
                result->token.literal.flt = a.token.literal.flt * (float)b.token.literal.num;
                result->token_type = DTYPE_FLT;
            } else error_code = 1;
        } else error_code = 1;
        break;
    case OP_DIV:
        if ((b.token_type == DTYPE_FLT && b.token.literal.flt == 0) || (b.token_type == DTYPE_NUM && b.token.literal.num == 0)) {
            error_code = 3;
            break;
        }
        if (a.token_type == DTYPE_NUM) {
            if (b.token_type == DTYPE_NUM) {
                //INT / INT -> INT
                result->token.literal.num = a.token.literal.num / b.token.literal.num;
                result->token_type = DTYPE_NUM;
            } else if (b.token_type == DTYPE_FLT) {
                //INT / FLOAT -> FLOAT
                result->token.literal.flt = (float)a.token.literal.num / b.token.literal.flt;
                result->token_type = DTYPE_FLT;
            } else error_code = 1;
        } else if (a.token_type == DTYPE_FLT) {
            if (b.token_type == DTYPE_FLT) {
                //FLOAT / FLOAT -> FLOAT
                result->token.literal.flt = a.token.literal.flt / b.token.literal.flt;
                result->token_type = DTYPE_FLT;
            } else if (b.token_type == DTYPE_NUM) {
                //FLOAT / INT -> FLOAT
                result->token.literal.flt = a.token.literal.flt / (float)b.token.literal.num;
                result->token_type = DTYPE_FLT;
            } else error_code = 1;
        } else error_code = 1;
        break;
    case OP_MOD:
        //Modulo is only INT % INT
        if (a.token_type == DTYPE_NUM) {
            if (b.token_type == DTYPE_NUM) {
                if (b.token.literal.num == 0) {
                    error_code = 3;
                    break;
                }
                result->token.literal.num = a.token.literal.num % b.token.literal.num;
                result->token_type = DTYPE_NUM;
            } else error_code = 1;
        } else error_code = 1;
        break;
    case OP_EQ:
        result->token_type = DTYPE_NUM;
        switch (a.token_type) {
        case DTYPE_NUM:
            //Cast float to integer
            if (b.token_type == DTYPE_NUM)
                result->token.literal.num = a.token.literal.num == b.token.literal.num;
            else if (b.token_type == DTYPE_FLT)
                result->token.literal.num = a.token.literal.num == (int)b.token.literal.flt;
            else
                result->token.literal.num = 0;
            break;
        case DTYPE_FLT:
            //Compare two floats directly
            if (b.token_type == DTYPE_FLT)
                result->token.literal.num = a.token.literal.flt == b.token.literal.flt;
            else if (b.token_type == DTYPE_FLT)
                result->token.literal.num = (int)a.token.literal.flt == b.token.literal.num;
            else
                result->token.literal.num = 0;
            break;
        case DTYPE_STR:
            //Compare if two strings are equal
            if (b.token_type == DTYPE_STR)
                result->token.literal.num = strcmp(a.token.literal.str, b.token.literal.str) == 0;
            else
                result->token.literal.num = 0;
            break;
        default:
            error_code = 1;
        }
        break;
    case OP_GT:
        result->token_type = DTYPE_NUM;
        switch (a.token_type) {
        case DTYPE_NUM:
            if (b.token_type == DTYPE_NUM)
                result->token.literal.num = a.token.literal.num > b.token.literal.num;
            else if (b.token_type == DTYPE_FLT)
                result->token.literal.num = a.token.literal.num > (int)b.token.literal.flt;
            else error_code = 1;
            break;
        case DTYPE_FLT:
            if (b.token_type == DTYPE_FLT)
                result->token.literal.num = a.token.literal.flt > b.token.literal.flt;
            else if (b.token_type == DTYPE_FLT)
                result->token.literal.num = (int)a.token.literal.flt > b.token.literal.num;
            else error_code = 1;
            break;
        default:
            error_code = 1;
        }
        break;
    case OP_LT:
        result->token_type = DTYPE_NUM;
        switch (a.token_type) {
        case DTYPE_NUM:
            if (b.token_type == DTYPE_NUM)
                result->token.literal.num = a.token.literal.num < b.token.literal.num;
            else if (b.token_type == DTYPE_FLT)
                result->token.literal.num = a.token.literal.num < (int)b.token.literal.flt;
            else error_code = 1;
            break;
        case DTYPE_FLT:
            if (b.token_type == DTYPE_FLT)
                result->token.literal.num = a.token.literal.flt < b.token.literal.flt;
            else if (b.token_type == DTYPE_FLT)
                result->token.literal.num = (int)a.token.literal.flt < b.token.literal.num;
            else error_code = 1;
            break;
        default:
            error_code = 1;
        }
        break;
    default:
        error_code = 2;
    }
    return error_code;
}

//Calculates Greater (in value) of the two given data
int ast_get_greater(ASTNodeData a, ASTNodeData b, ASTNodeData *result) {
    int error_code = 0;
    *result = ASTVOID;
    ASTNodeData bigger;

    //Figure out which is the larger operand
    switch (a.token_type) {
    case DTYPE_NUM:
        switch (b.token_type) {
        case DTYPE_NUM:
            bigger = a.token.literal.num >= b.token.literal.num ? a : b;
            break;
        case DTYPE_FLT:
            bigger = (float)(a.token.literal.num) >= b.token.literal.flt ? a : b;
            break;
        case DTYPE_NONE:
            bigger = a;
            break;
        default:
            error_code = 1;
        }
        break;
    case DTYPE_FLT:
        switch (b.token_type) {
        case DTYPE_NUM:
            bigger = a.token.literal.flt >= (float)(b.token.literal.num) ? a : b;
            break;
        case DTYPE_FLT:
            bigger = a.token.literal.flt >= b.token.literal.flt ? a : b;
            break;
        case DTYPE_NONE:
            bigger = a;
            break;
        default:
            error_code = 1;
        }
        break;
    case DTYPE_NONE:
        bigger = b;
        break;
    default:
        error_code = 1;
    }

    if (error_code == 0) {
        result->token_type = bigger.token_type;
        switch (bigger.token_type) {
        case DTYPE_NUM:
            result->token.literal.num = bigger.token.literal.num;
            break;
        case DTYPE_FLT:
            result->token.literal.flt = bigger.token.literal.flt;
            break;
        }
    }

    return error_code;
}

//Calculates Lesser (in value) of the two given data
int ast_get_lesser(ASTNodeData a, ASTNodeData b, ASTNodeData *result) {
    int error_code = 0;
    *result = ASTVOID;
    ASTNodeData smaller;

    //Figure out which is the larger operand
    switch (a.token_type) {
    case DTYPE_NUM:
        switch (b.token_type) {
        case DTYPE_NUM:
            smaller = a.token.literal.num <= b.token.literal.num ? a : b;
            break;
        case DTYPE_FLT:
            smaller = (float)(a.token.literal.num) <= b.token.literal.flt ? a : b;
            break;
        case DTYPE_NONE:
            smaller = a;
            break;
        default:
            error_code = 1;
        }
        break;
    case DTYPE_FLT:
        switch (b.token_type) {
        case DTYPE_NUM:
            smaller = a.token.literal.flt <= (float)(b.token.literal.num) ? a : b;
            break;
        case DTYPE_FLT:
            smaller = a.token.literal.flt <= b.token.literal.flt ? a : b;
            break;
        case DTYPE_NONE:
            smaller = a;
            break;
        default:
            error_code = 1;
        }
        break;
    case DTYPE_NONE:
        smaller = b;
        break;
    default:
        error_code = 1;
    }

    if (error_code == 0) {
        result->token_type = smaller.token_type;
        switch (smaller.token_type) {
        case DTYPE_NUM:
            result->token.literal.num = smaller.token.literal.num;
            break;
        case DTYPE_FLT:
            result->token.literal.flt = smaller.token.literal.flt;
            break;
        }
    }

    return error_code;
}
