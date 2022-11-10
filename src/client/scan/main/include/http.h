#ifndef HTTP_H_
#define HTTP_H_

#define MAX_HTTP_RECV_BUFFER_SIZE 512
#define MAX_HTTP_OUTPUT_BUFFER_SIZE 2048

void http_init();
void http_request_url();

#endif // HTTP_H_
