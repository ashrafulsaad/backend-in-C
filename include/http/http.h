#ifndef HTTP_H
#define HTTP_H

#include "server.h"

int http_parse_request(const char *input, struct HttpRequest *request);
void http_prepare_response(struct HttpResponse *response, int status_code, const char *reason_phrase, const char *content_type, const char *body);
const char *http_mime_for_path(const char *path);

#endif
