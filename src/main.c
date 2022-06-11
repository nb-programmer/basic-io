
/**
* Program: Cloud BASIC parser and interpreter, server program.
* Author: Team 4 - Roll no. 2004
* Subject: PL206 Project, FY SEM II
* 
* Description:
* Starts an HTTP server that has an endpoint for executing BASIC program
* by lexing, parsing and executing the code. The server program serves
* HTML files for loading the UI page.
*/

/*
References:
https://www.youtube.com/watch?v=t4e7PjRygt0
https://www.informit.com/articles/article.aspx?p=28697&seqNum=4
https://www.tutorialspoint.com/c_standard_library/
https://simplesnippets.tech/infix-to-prefix-conversion-using-stack-data-structure-with-c-program-code/
https://en.cppreference.com/w/c/language/operator_precedence
*/

#include "utils.h"
#include "http.h"
#include "tcpserver.h"
#include "ast.h"
#include "basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

//Linux libraries for fork() call
#include <sys/types.h> 
#include <unistd.h>
//For open(), write() and constants
#include <fcntl.h>
#include <sys/stat.h>

//Comment to print output to console only
#define OUTPUT_CLIENT_REDIRECT

//#define TEST_AST
//#define TEST_PARSE

//Test AST builder functions
#ifdef TEST_AST
void test_build_hello(ASTNode *program_sequence);
void test_build_var_print(ASTNode *program_sequence);
void test_build_expr_print(ASTNode *program_sequence);
#endif

//Which TCP Port to start listening on
const int LISTEN_PORT = 1111;

/* APPLICATION FUNCTIONS */

void program_parse_and_run(char *buffer) {
    BASICProgram *program;
    BASICRuntime *runtime;

    program = basic_create_program();
    if (program == NULL) {
        lprintf("RUN", LOGTYPE_ERROR, "Failed to initialize basic program\n");
        return;
    }

    //Source code bind
    program->program_source = buffer;

    lprintf("RUN", LOGTYPE_MESSAGE, "Lexing BASIC program\n");
    if (basic_tokenize(program) == 0) {
        lprintf("RUN", LOGTYPE_MESSAGE, "Parsing to AST\n");
        if (basic_parse_to_ast(program) == 0) {
            lprintf("RUN", LOGTYPE_MESSAGE, "Preparing runtime\n");
            runtime = basic_create_runtime(program);
            if (runtime == NULL) {
                lprintf("RUN", LOGTYPE_ERROR, "Failed to create a BASIC runtime object\n");
            } else {
                //Execute the BASIC program from first instruction in the sequence
                lprintf("RUN", LOGTYPE_MESSAGE, "Running BASIC program\n");
                basic_execute(runtime, program->program_sequence);
                lprintf("RUN", LOGTYPE_MESSAGE, "Program finished executing\n");
            }
        }
    } else {
        lprintf("RUN", LOGTYPE_ERROR, "Failed to parse the program\n");
    }

    //Cleanup
    basic_free_runtime(runtime);
    basic_destroy_program(program);
}

//Prepares a buffer to store incoming program source code
void alloc_read_http_program(int sock_fd, http_request_header *req, http_response_header *res, char **buffer) {
    if (req->method != METHOD_POST) {
        //Unsupported method, only POST will be processed
        res->status_code = 405;
        http_write_header(sock_fd, res);
        fd_write_string(sock_fd, "Method not supported.\nThis URI only supports POST method.");
        return;
    }

    if (req->content_length < 0) {
        //Need to send content length header to read data
        res->status_code = 411;
        http_write_header(sock_fd, res);
        return;
    }

    if (req->content_length > (10 * 1048576)) {
        //Limit size to 10MB
        res->status_code = 403;
        http_write_header(sock_fd, res);
        return;
    }

    //Allocate some area for the input program, + 1 byte for null-terminator
    //calloc() call makes sure that the data is always null-terminated
    int buffer_size = MAX(16, req->content_length) + 1;
    *buffer = (char *)calloc(buffer_size, sizeof(char));
    if (*buffer == NULL) {
        //Failed to allocate space
        res->status_code = 500;
        http_write_header(sock_fd, res);
        fd_write_string(sock_fd, "Failed to allocate buffer to save program");
        return;
    }

    //Try to read the requested data
    if (http_read_body(sock_fd, req, *buffer) < 0) {
        res->status_code = 500;
        http_write_header(sock_fd, res);
        fd_write_string(sock_fd, "Failed to read the program from client request");
        free(*buffer);
        *buffer = NULL;
        return;
    }
}

//Route for POST "/execute"
void run_basic_program(int sock_fd, http_request_header *req, http_response_header *res) {
    int show_parser_log = 0, show_runner_log = 0;
    char *buffer = NULL;
    alloc_read_http_program(sock_fd, req, res, &buffer);
    if (buffer == NULL) return;

    //Check if any flags were passed
    for (int i=0;i<req->query_string_count;i++) {
        char *query = req->query_string[i];
        printf("[HTTP] > Query flag get: %s\n", query);
        if (strcmp(query, "show_parser_log") == 0) show_parser_log = 1;
        if (strcmp(query, "show_runner_log") == 0) show_runner_log = 1;
    }

    /* Debugging */

    //Reset logging mask
    log_print_mask = LOGTYPE_ERROR | LOGTYPE_INFO;

    if (show_parser_log) {
        //Parser logs to "debug" mask
        log_print_mask |= LOGTYPE_DEBUG;
    }
    if (show_runner_log) {
        //Runner logs to "message" mask
        log_print_mask |= LOGTYPE_MESSAGE;
    }

    //Put out OK header
    res->status_code = 200;
    http_write_header(sock_fd, res);

    printf("[HTTP] Beginning executing the program\n");

    //Redirect all output from this point onwards to the client
#ifdef OUTPUT_CLIENT_REDIRECT
    int copy_stdout = stdout2fd_set(sock_fd);
#endif
    //Now, anything written to stdout will be sent to the client

    //Run the basic program. Any output produced is sent directly to the client
    program_parse_and_run(buffer);

    //Write out any remaining stream data
    fflush(stdout);

    //Free all memory buffers
    free(buffer);

    //Restore stdout back to normal
#ifdef OUTPUT_CLIENT_REDIRECT
    stdout2fd_reset(copy_stdout);
#endif
    printf("[HTTP] Done executing the program\n");
}


//This function is of type http_resp_cb
//Respond to client requests based on URI
void generate_response(int sock_fd, http_request_header *req, http_response_header *res) {
    const char path_execute[] = "/execute";
    int arg1_int;

    if (strcmp(req->fetch_path, "/") == 0) {
        //Landing page
        http_respond_file(sock_fd, res, "static/index.html", "text/html");
    } else if (strcmp(req->fetch_path, "/codemirror.js") == 0) {
        //CodeMirror main JS
        http_respond_file(sock_fd, res, "static/codemirror.js", "text/javascript");
    } else if (strcmp(req->fetch_path, "/codemirror.css") == 0) {
        //CodeMirror main css
        http_respond_file(sock_fd, res, "static/codemirror.css", "text/css");
    } else if (strcmp(req->fetch_path, "/basic.js") == 0) {
        //CodeMirror BASIC syntax module
        http_respond_file(sock_fd, res, "static/basic.js", "text/javascript");
    } else if (strcmp(req->fetch_path, path_execute) == 0) {
        //Execute given BASIC program
        run_basic_program(sock_fd, req, res);
    } else {
        //Send a 404 for everything else
        http_respond_status_nf(sock_fd, res);
    }
}

//This function is of type tcpserver_handler
//Spawn a process to handle the HTTP request
int spawn_http_worker(int cli_sock, tcp_server *tcpsv) {
    //Child process has a reference to the listening and client socket
    //We need to close the client socket in the parent process
    //and close the listening socket in the child process.
    //After child process is done, exit promptly.
    //See https://stackoverflow.com/a/6019241/12887350

    int pid = fork();
    if (pid < 0) return pid;
    else if (pid == 0) {
        //Child process

        //Give the child a new RNG seed
        srand(time(0));

        //Close listening socket to decrement reference count
        close(tcpsv->listen_sock);

        //Handle the client request
        http_respond(cli_sock, generate_response);

        //Exit the child process
        _exit(0);
    } else {
        //Parent process

        //Close client socket to decrement reference count
        close(cli_sock);
        return 0;
    }

    return 0;
}

//Simply run the program sequence from the given program
//Used for testing only
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

/* MAIN PROGRAM ENTRY POINT */

int main(int argc, char *argv[]) {
    //TCP server object to listen to the specified port
	tcp_server sock_sv;
    sock_sv.listen_port = LISTEN_PORT;
    sock_sv.client_handler = spawn_http_worker;

    //Initialize an new seed based on current time
    srand(time(0));

#ifdef TEST_AST
    BASICProgram *basic_program;
    basic_program = basic_create_program();

    //test_build_hello(basic_program->program_sequence);
    //test_build_var_print(basic_program->program_sequence);
    test_build_expr_print(basic_program->program_sequence);
    printf("AST node structure:\n");
    ast_display(basic_program->program_sequence);
    printf("[MAIN] Begin interpreting program...\n");

    interpret_basic_program(basic_program);

    basic_destroy_program(basic_program);
#elif defined(TEST_PARSE)
    //Simple linear expression test
    const char test_prog[] = "PRINT(20 * 30 + 10)\nPRINT(10 + (10 - 1) + 10 - 1 / 2)";

    //Simple IF clause test
    const char test_prog_if[] = "X = TRUE\nIF X THEN\nPRINT(1)END\nPRINT(22)";
    
    //This one generates so many closing braces in the console, trippy!
    const char test_prog_if_nested[] = "IF 1 THEN\nIF 0 THEN PRINT(1) END END\n";

    const char test_prog_delay[] = "print(\"Test\")\nsleep(1)\nprint(\"Done\")\n";

    BASICProgram *basic_program = basic_create_program();

    //Place whatever program to test here
    basic_program->program_source = test_prog_delay;

    basic_tokenize(basic_program);
    basic_parse_to_ast(basic_program);
    printf("AST node structure:\n");
    ast_display(basic_program->program_sequence);
    interpret_basic_program(basic_program);
    basic_destroy_program(basic_program);
#else
    /* Main application code is here! */
    if (tcpserver_create(&sock_sv) != 0)
        return 1;

    //Start the TCP server, waiting for new clients to connect
    if (tcpserver_startlistening(&sock_sv) != 0)
        return 1;

    tcpserver_close(&sock_sv);
#endif

    return 0;
}