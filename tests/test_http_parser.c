#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "server.h"

int main(void)
{
    struct HttpRequest request;
    const char *raw = "GET /health?name=alice HTTP/1.1\r\nHost: localhost\r\nUser-Agent: test\r\n\r\n";

    int rc = http_parse_request(raw, &request);
    assert(rc == 0);
    assert(strcmp(request.method, "GET") == 0);
    assert(strcmp(request.path, "/health") == 0);
    assert(strcmp(request.query, "name=alice") == 0);
    assert(request.header_count == 2);
    assert(strcmp(request.headers[0].name, "Host") == 0);
    assert(strcmp(request.headers[1].name, "User-Agent") == 0);

    const char *raw_post = "POST /api/echo HTTP/1.1\r\nHost: localhost\r\nContent-Length: 7\r\n\r\nhello=1";
    rc = http_parse_request(raw_post, &request);
    assert(rc == 0);
    assert(strcmp(request.method, "POST") == 0);
    assert(request.body_length == 7);
    assert(strcmp(request.body, "hello=1") == 0);

    puts("http_parse_request smoke test passed");
    return 0;
}
