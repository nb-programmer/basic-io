
// Standard libraries
#include <string.h>

#include <basic/ast.h>
#include <basic/basic.h>

// Simply run the program sequence from the given program
// Used for testing only
void interpret_basic_program(BASICProgram *program) {
    StringLiteral buffer;
    BASICRuntime *runtime;

    runtime = basic_create_runtime(program);
    if (runtime == NULL) return;
    ASTNodeData ret_val = basic_execute(runtime, program->program_sequence);
    ast_data_as_string(ret_val, buffer);
    basic_free_runtime(runtime);
    printf("[EXEC] Program finished with result: %s\n", buffer);
}


// Functions to build example programs in the form of ASTs

/**
 * Test program to generate a simple "Hello, world!" program tree 
 * 
 * Tree Structure:
 * 1)
 *       (PRINT)
 *          |        
 *   ("Hello, world!")
 * 
 */
void test_build_hello(ASTNode *program_sequence) {
    // PRINT - Function call
    ASTNode *node_fn_print = ast_create_node();
    node_fn_print->type = AST_FUNC_CALL;
    strcpy(node_fn_print->data.token.kw, "PRINT");
    ast_append_child(program_sequence, node_fn_print);

    // "Hello, world!" - String literal
    ASTNode *node_fn_print_arg = ast_create_node();
    node_fn_print_arg->type = AST_IMMEDIATE;
    strcpy(node_fn_print_arg->data.token.literal.str, "Hello, world!");
    node_fn_print_arg->data.token_type = DTYPE_STR;

    ast_append_child(node_fn_print, node_fn_print_arg);
}

/**
 * Simple test program to assign a variable a value, and print it
 * 
 * Tree Structure:
 * 1)
 *     (Assignment)
 *       /      \
 *     (A)      (5)
 * 
 * 2)
 *        (    PRINT    )
 *         /           \
 *     ("Answer is")   (A)
 * 
 */
void test_build_var_print(ASTNode *program_sequence) {
    // To assign "A = 5"
    
    // Operator +
    ASTNode *node_asn_var_a = ast_create_node();
    node_asn_var_a->type = AST_OPERATION;
    node_asn_var_a->data.token.op = OP_ASSIGN;
    node_asn_var_a->data.token_type = DTYPE_SYMB;
    ast_append_child(program_sequence, node_asn_var_a);

    // "A" - Variable name
    ASTNode *node_param_var_a = ast_create_node();
    node_param_var_a->type = AST_VARIABLE;
    strcpy(node_param_var_a->data.token.variable_name, "A");
    node_param_var_a->data.token_type = DTYPE_SYMB;
    ast_append_child(node_asn_var_a, node_param_var_a);

    // "5" - Integer literal
    ASTNode *node_param_val_2 = ast_create_node();
    node_param_val_2->type = AST_IMMEDIATE;
    node_param_val_2->data.token.literal.num = 5;
    node_param_val_2->data.token_type = DTYPE_NUM;
    ast_append_child(node_asn_var_a, node_param_val_2);

    // To print the value of "A"

    // PRINT - Function call
    ASTNode *node_fn_print = ast_create_node();
    node_fn_print->type = AST_FUNC_CALL;
    strcpy(node_fn_print->data.token.kw, "PRINT");
    ast_append_child(program_sequence, node_fn_print);

    // "Answer is" - String literal
    ASTNode *node_fn_print_arg_msg = ast_create_node();
    node_fn_print_arg_msg->type = AST_IMMEDIATE;
    strcpy(node_fn_print_arg_msg->data.token.literal.str, "Answer is");
    node_fn_print_arg_msg->data.token_type = DTYPE_STR;
    ast_append_child(node_fn_print, node_fn_print_arg_msg);

    // Parameter - Variable "A"
    ASTNode *node_fn_print_arg_var = ast_create_node();
    node_fn_print_arg_var->type = AST_VARIABLE;
    strcpy(node_fn_print_arg_var->data.token.variable_name, "A");
    node_fn_print_arg_var->data.token_type = DTYPE_SYMB;
    ast_append_child(node_fn_print, node_fn_print_arg_var);
}


/**
 * A bit more complex program to calculate and print an expression
 * 
 * Tree Structure:
 * 1)
 *     (Assignment)
 *       /      \
 *     (A)      (5)
 * 
 * 2)
 *     (Assignment)
 *       /      \
 *     (B)      (6)
 * 
 * 3)
 *       (                   PRINT                   )
 *           /          |     |     |     |       \
 *     ("Answer for")  (A)  ("+")  (B)  ("=")  (Operation +)
 *                                                /      \
 *                                               (A)     (B)
 * 
 */
void test_build_expr_print(ASTNode *program_sequence) {
    // To assign "A = 5"
    ASTNode *node_asn_var_a = ast_create_node();
    node_asn_var_a->type = AST_OPERATION;
    node_asn_var_a->data.token.op = OP_ASSIGN;
    node_asn_var_a->data.token_type = DTYPE_SYMB;
    ast_append_child(program_sequence, node_asn_var_a);

    // "A" - Variable name
    ASTNode *node_param_var_a = ast_create_node();
    node_param_var_a->type = AST_VARIABLE;
    strcpy(node_param_var_a->data.token.variable_name, "A");
    node_param_var_a->data.token_type = DTYPE_SYMB;
    ast_append_child(node_asn_var_a, node_param_var_a);

    // "5" - Literal value
    ASTNode *node_param_val_1 = ast_create_node();
    node_param_val_1->type = AST_IMMEDIATE;
    node_param_val_1->data.token.literal.num = 5;
    node_param_val_1->data.token_type = DTYPE_NUM;
    ast_append_child(node_asn_var_a, node_param_val_1);

    // To assign "B = 6"
    ASTNode *node_asn_var_b = ast_create_node();
    node_asn_var_b->type = AST_OPERATION;
    node_asn_var_b->data.token.op = OP_ASSIGN;
    node_asn_var_b->data.token_type = DTYPE_SYMB;
    ast_append_child(program_sequence, node_asn_var_b);

    // "B" - Variable name
    ASTNode *node_param_var_b = ast_create_node();
    node_param_var_b->type = AST_VARIABLE;
    strcpy(node_param_var_b->data.token.variable_name, "B");
    node_param_var_b->data.token_type = DTYPE_SYMB;
    ast_append_child(node_asn_var_b, node_param_var_b);

    // "6" - Literal value
    ASTNode *node_param_val_2 = ast_create_node();
    node_param_val_2->type = AST_IMMEDIATE;
    node_param_val_2->data.token.literal.num = 6;
    node_param_val_2->data.token_type = DTYPE_NUM;
    ast_append_child(node_asn_var_b, node_param_val_2);

    // To print the value of "A + B"

    // PRINT - Function call
    ASTNode *node_fn_print = ast_create_node();
    node_fn_print->type = AST_FUNC_CALL;
    strcpy(node_fn_print->data.token.kw, "PRINT");
    ast_append_child(program_sequence, node_fn_print);

    // "Answer for" - String literal
    ASTNode *node_fn_print_arg_msg = ast_create_node();
    node_fn_print_arg_msg->type = AST_IMMEDIATE;
    strcpy(node_fn_print_arg_msg->data.token.literal.str, "Answer for");
    node_fn_print_arg_msg->data.token_type = DTYPE_STR;
    ast_append_child(node_fn_print, node_fn_print_arg_msg);

    // Parameter - Variable A
    ASTNode *node_param_pr_var_a = ast_create_node();
    node_param_pr_var_a->type = AST_VARIABLE;
    strcpy(node_param_pr_var_a->data.token.variable_name, "A");
    node_param_pr_var_a->data.token_type = DTYPE_SYMB;
    ast_append_child(node_fn_print, node_param_pr_var_a);

    // Parameter - String "+"
    ASTNode *node_param_pr_add = ast_create_node();
    node_param_pr_add->type = AST_IMMEDIATE;
    strcpy(node_param_pr_add->data.token.literal.str, "+");
    node_param_pr_add->data.token_type = DTYPE_STR;
    ast_append_child(node_fn_print, node_param_pr_add);

    // Parameter - Variable B
    ASTNode *node_param_pr_var_b = ast_create_node();
    node_param_pr_var_b->type = AST_VARIABLE;
    strcpy(node_param_pr_var_b->data.token.variable_name, "B");
    node_param_pr_var_b->data.token_type = DTYPE_SYMB;
    ast_append_child(node_fn_print, node_param_pr_var_b);

    // Parameter - String "="
    ASTNode *node_param_pr_equals = ast_create_node();
    node_param_pr_equals->type = AST_IMMEDIATE;
    strcpy(node_param_pr_equals->data.token.literal.str, "=");
    node_param_pr_equals->data.token_type = DTYPE_STR;
    ast_append_child(node_fn_print, node_param_pr_equals);

    // Parameter - Operation +
    ASTNode *node_fn_print_arg_op = ast_create_node();
    node_fn_print_arg_op->type = AST_OPERATION;
    node_fn_print_arg_op->data.token.op = OP_ADD;
    node_fn_print_arg_op->data.token_type = DTYPE_SYMB;
    ast_append_child(node_fn_print, node_fn_print_arg_op);

    // Parameter 1 - First operand "A"
    ASTNode *node_param_op_var_a = ast_create_node();
    node_param_op_var_a->type = AST_VARIABLE;
    strcpy(node_param_op_var_a->data.token.variable_name, "A");
    node_param_op_var_a->data.token_type = DTYPE_SYMB;
    ast_append_child(node_fn_print_arg_op, node_param_op_var_a);

    // Parameter 2 - Second operand "B"
    ASTNode *node_param_op_var_b = ast_create_node();
    node_param_op_var_b->type = AST_VARIABLE;
    strcpy(node_param_op_var_b->data.token.variable_name, "B");
    node_param_op_var_b->data.token_type = DTYPE_SYMB;
    ast_append_child(node_fn_print_arg_op, node_param_op_var_b);
}

/*
    // test_build_hello(basic_program->program_sequence);
    // test_build_var_print(basic_program->program_sequence);
    test_build_expr_print(basic_program->program_sequence);
    printf("AST node structure:\n");
    ast_display(basic_program->program_sequence);
    printf("[MAIN] Begin interpreting program...\n");

    interpret_basic_program(basic_program);

    basic_destroy_program(basic_program);
*/

int main(int argc, char *argv[]) {
    BASICProgram *basic_program;
    basic_program = basic_create_program();

    // Simple linear expression test
    const char test_prog[] = "PRINT(20 * 30 + 10)\nPRINT(10 + (10 - 1) + 10 - 1 / 2)";

    // Simple IF clause test
    const char test_prog_if[] = "X = TRUE\nIF X THEN\nPRINT(1)END\nPRINT(22)";
    
    // This one generates so many closing braces in the console, trippy!
    const char test_prog_if_nested[] = "IF 1 THEN\nIF 0 THEN PRINT(1) END END\n";

    const char test_prog_delay[] = "print(\"Test\")\nsleep(1)\nprint(\"Done\")\n";

    const char test_prog_fn_call_in_fn_call[] = "print(random() * 1000)\n";

    // Place whatever program to test here
    basic_program->program_source = test_prog_fn_call_in_fn_call;

    basic_tokenize(basic_program);
    basic_parse_to_ast(basic_program);
    printf("AST node structure:\n");
    ast_display(basic_program->program_sequence);
    interpret_basic_program(basic_program);
    basic_destroy_program(basic_program);
}
