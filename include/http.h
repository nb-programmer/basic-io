#ifndef _HTTP_H
#define _HTTP_H

//Which request method was used
typedef enum {
    METHOD_UNKNOWN,
    METHOD_GET,
    METHOD_POST
} HTTPMETHOD;

//Request header parameters passed by the Web Browser
typedef struct _http_req_header {
    char *buffer;
    int buffer_len;
    int data_start, data_read_len;

    char *header_string;
    HTTPMETHOD method;
    char fetch_path[256];
    int content_length;
    char query_string[4][256];
    int query_string_count;
} http_request_header;

//Header parameters to send back to the Web Browser as a response
typedef struct _http_resp_header {
    unsigned int status_code;
    char content_type[64];
} http_response_header;

//Callback function to generate a response for each request
typedef void(*http_resp_cb)(int sock_fd, http_request_header *req, http_response_header *res);


//Public functions
void http_write_header(int sock_fd, http_response_header *header);
void http_respond(int sock_fd, http_resp_cb http_response_generate);
int http_read_body(int sock_fd, http_request_header *req, char *buffer);
void http_respond_file(int sock_fd, http_response_header *res, char *path, const char *mime_type);
void http_respond_status_nf(int sock_fd, http_response_header *res);

//Private functions
void _http_get_status(int status_code, char *out_status);
void _http_parse_request_header(http_request_header *header);
void _http_parse_query_parameters(char *tmp, http_request_header *header);

#endif