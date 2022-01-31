
#include "http.h"
#include "utils.h"

//Standard libraries
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Linux file IO libraries
#include <fcntl.h>
#include <unistd.h>

//Linux Socket libraries
#include <sys/socket.h>


/* Public functions */

//Simple HTTP response header
void http_write_header(int sock_fd, http_response_header *header) {
    char status_msg[128], buff[256];
    _http_get_status(header->status_code, status_msg);
    sprintf(buff, "HTTP/1.1 %d %s\r\n", header->status_code, status_msg);
    fd_write_string(sock_fd, buff);
    sprintf(buff, "Content-Type: %s\r\n", header->content_type);
    fd_write_string(sock_fd, buff);
    fd_write_string(sock_fd, "Server: C server\r\n");
    fd_write_string(sock_fd, "\r\n");
}

//Read request header, and call a callback function to generate the response
void http_respond(int sock_fd, http_resp_cb http_response_generate) {
    http_request_header req_hdr;
    http_response_header res_hdr;

    char input_chunk_buffer[128];
    int buffer_pos = 0, read_len;

    req_hdr.buffer_len = sizeof(input_chunk_buffer);
    req_hdr.data_start = 0;
    req_hdr.data_read_len = 0;

    req_hdr.buffer = (char *)malloc(req_hdr.buffer_len);
    memset(req_hdr.buffer, 0, req_hdr.buffer_len);
    req_hdr.header_string = NULL;
    req_hdr.query_string_count = 0;

    while (1) {
        read_len = recv(sock_fd, input_chunk_buffer, sizeof(input_chunk_buffer), 0);
        if (read_len <= 0) {
            fprintf(stderr, "Socket unexpectedly closed while reading data\n");
            break;
        }

        req_hdr.data_read_len += read_len;
        //Resize buffer if needed
        if (req_hdr.buffer_len - buffer_pos < read_len) {
            req_hdr.buffer_len += read_len * 2;
            req_hdr.buffer = (char *)realloc(req_hdr.buffer, req_hdr.buffer_len);
        }

        memcpy(req_hdr.buffer + buffer_pos, input_chunk_buffer, read_len);
        buffer_pos += read_len;

        //Check if end of header is reached
        int is_header_end = 0;
        if (req_hdr.data_read_len >= 4) {
            //Search from end the pattern of header end
            for (int i=req_hdr.data_read_len;i>=4;i--) {
                if (memcmp(req_hdr.buffer + i - 4, "\r\n\r\n", 4) == 0) {
                    is_header_end = 1;
                    req_hdr.data_start = i;
                    break;
                }
            }
        }
        if (is_header_end) {
            req_hdr.header_string = (char *)malloc(req_hdr.data_start + 1);
            memcpy(req_hdr.header_string, req_hdr.buffer, req_hdr.data_start);
            req_hdr.header_string[req_hdr.data_start] = '\0';
            break;
        }
    }

    //At this point, req_hdr.buffer contains the HTTP header + partial content of the body, if sent.
    //req_hdr.header_string is just the HTTP header.

    //Parse the header, and then free up the dynamic memory as it is not needed anymore
    _http_parse_request_header(&req_hdr);
    free(req_hdr.header_string);

    //Default header
    res_hdr.status_code = 200;
    strcpy(res_hdr.content_type, "text/plain");

    //Call the HTTP response generator
    //TODO: Handle nullptr handler function with stub function
    http_response_generate(sock_fd, &req_hdr, &res_hdr);

    free(req_hdr.buffer);
    close(sock_fd);
}

//Reads the http body into given buffer, upto "Content-Length" bytes
int http_read_body(int sock_fd, http_request_header *req, char *buffer) {
    int total_read = 0;
    if (req->content_length > 0) {
        int data_extra_read = req->data_read_len - req->data_start;
        memcpy(buffer, req->buffer + req->data_start, data_extra_read);
        total_read += data_extra_read;
        
        //If still more data is remaining, continue receiving
        int remaining_bytes = req->content_length - data_extra_read;
        if (remaining_bytes > 0) {
            printf("Reading more %d bytes\n", remaining_bytes);
            total_read += recv(sock_fd, buffer + data_extra_read, remaining_bytes, 0);
        }
    }
    return total_read;
}

//Generate response data by reading a file
void http_respond_file(int sock_fd, http_response_header *res, char *path, const char *mime_type) {
    int file_fd;
    char buffer[4096];
    int read_size;

    if ((file_fd = open(path, O_RDONLY)) < 0) {
        http_respond_status_nf(sock_fd, res);
        return;
    }

    res->status_code = 200;
    strcpy(res->content_type, mime_type);
    http_write_header(sock_fd, res);

    while ((read_size = read(file_fd, buffer, sizeof(buffer))) > 0)
        send(sock_fd, buffer, read_size, 0);

    close(file_fd);
}

//Generate 404 response
void http_respond_status_nf(int sock_fd, http_response_header *res) {
    res->status_code = 404;
    strcpy(res->content_type, "text/plain");
    http_write_header(sock_fd, res);
    fd_write_string(sock_fd, "Requested file was not found.");
}


/* Private functions */

void _http_parse_query_parameters(char *tmp, http_request_header *header) {
    char *path_line, *path_ctx = tmp;
    //tmp gets cut off till the query string automatically by strtok_r()
    if ((path_line = strtok_r(path_ctx, "?", &path_ctx))) {
        //Go through each query parameter
        while ((path_line = strtok_r(path_ctx, "&", &path_ctx)))
            strcpy(header->query_string[header->query_string_count++], path_line);
    }
}

//Convert status code to string
void _http_get_status(int status_code, char *out_status) {
    switch (status_code) {
    //Client-side errors
    case 403:
        strcpy(out_status, "Forbidden");
        break;
    case 404:
        strcpy(out_status, "Not Found");
        break;
    case 405:
        strcpy(out_status, "Method Not Allowed");
        break;
    case 411:
        strcpy(out_status, "Length Required");
        break;

    //Server-side errors
    case 500:
        strcpy(out_status, "Internal Server Error");
        break;

    case 200:
    default:
        strcpy(out_status, "OK");
    }
}

//Find tokens in the HTTP header string and set the corresponding header fields
void _http_parse_request_header(http_request_header *header) {
    //For each line in the header
    char* line_token, *line_ctx = header->header_string;
    int fname_start, fname_end;
    int is_header_head = 1;

    //Breaking in the Header line
    char *hdr_ctx, *hdr_key, *hdr_value;

    //Temporary buffer
    char tmp[256];

    //Assume default values
    header->content_length = -1;
    header->method = METHOD_UNKNOWN;

    //First split string by \r\n lines
    while ((line_token = strtok_r(line_ctx, "\r\n", &line_ctx))) {
        //printf("Header line: %s\n", line_token);

        //If this is the first line, aka the method and URI
        if (strlen(line_token) >= 1) {
            if (is_header_head) {
                is_header_head = 0;
                if (strncmp(line_token, "GET", 3) == 0) {
                    //If it is a GET request, find the URI and mark as "GET"
                    fname_start = 4;
                    fname_end = strlen(line_token) - 9;    //size of " HTTP/1.1"
                    memcpy(tmp, line_token + fname_start, fname_end - fname_start);
                    tmp[fname_end - fname_start] = '\0';
                    _http_parse_query_parameters(tmp, header);
                    strcpy(header->fetch_path, tmp);
                    header->method = METHOD_GET;
                } else if (strncmp(line_token, "POST", 4) == 0) {
                    //If it is a POST request, find the URI and mark as "POST"
                    fname_start = 5;
                    fname_end = strlen(line_token) - 9;    //size of " HTTP/1.1"
                    memcpy(tmp, line_token + fname_start, fname_end - fname_start);
                    tmp[fname_end - fname_start] = '\0';
                    _http_parse_query_parameters(tmp, header);
                    strcpy(header->fetch_path, tmp);
                    header->method = METHOD_POST;
                }
            } else {
                //Rest of the headers, like Host, Content-Length, etc.
                hdr_ctx = line_token;
                //Split the header by a colon to get key / value
                hdr_key = strtok_r(hdr_ctx, ":", &hdr_ctx);
                if (hdr_key == NULL) continue;
                hdr_value = strtok_r(hdr_ctx, ":", &hdr_ctx);
                if (hdr_value != NULL) {
                    //Remove one space character after colon
                    if (*hdr_value == ' ')
                        hdr_value++;
                }

                //Determine what header we got
                if (strcasecmp(hdr_key, "Content-Length") == 0)
                    header->content_length = atoi(hdr_value);
            }
        }
    }
}