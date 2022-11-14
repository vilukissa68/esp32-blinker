#ifndef HTTP_H_
#define HTTP_H_

#define MAX_HTTP_RECV_BUFFER_SIZE 512
#define MAX_HTTP_OUTPUT_BUFFER_SIZE 2048

typedef struct {
    void* ready_handler;
} http_t;

typedef struct {
    char * key;
    char * value;
    int timestamp;
} http_ready_t;

void http_init();
void http_request_url();

#endif // HTTP_H_
