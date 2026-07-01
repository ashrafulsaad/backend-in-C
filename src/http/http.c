#include "http/http.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void http_set_header(struct HttpRequest *request, const char *name, const char *value)
{
    if (!request || request->header_count >= MAX_HEADERS) {
        return;
    }

    snprintf(request->headers[request->header_count].name, sizeof(request->headers[request->header_count].name), "%s", name);
    snprintf(request->headers[request->header_count].value, sizeof(request->headers[request->header_count].value), "%s", value);
    request->header_count++;
}

int http_parse_request(const char *input, struct HttpRequest *request)
{
    char buffer[MAX_BODY_SIZE];
    char *line = NULL;
    char *header_save = NULL;

    if (!input || !request) {
        return -1;
    }

    memset(request, 0, sizeof(*request));
    snprintf(buffer, sizeof(buffer), "%s", input);

    char *body_start = NULL;
    char *header_end = strstr(buffer, "\r\n\r\n");
    if (header_end) {
        *header_end = '\0';
        body_start = header_end + 4;
    } else {
        header_end = strstr(buffer, "\n\n");
        if (header_end) {
            *header_end = '\0';
            body_start = header_end + 2;
        }
    }

    line = strtok_r(buffer, "\r\n", &header_save);
    if (!line) {
        return -1;
    }

    char *method = strtok(line, " ");
    char *target = strtok(NULL, " ");
    char *version = strtok(NULL, " ");
    if (!method || !target || !version) {
        return -1;
    }

    char *query = strchr(target, '?');
    if (query) {
        *query++ = '\0';
        snprintf(request->query, sizeof(request->query), "%s", query);
    } else {
        request->query[0] = '\0';
    }

    snprintf(request->method, sizeof(request->method), "%s", method);
    snprintf(request->path, sizeof(request->path), "%s", target);
    snprintf(request->version, sizeof(request->version), "%s", version);

    size_t content_length = 0;
    while ((line = strtok_r(NULL, "\r\n", &header_save)) != NULL) {
        if (line[0] == '\0') {
            break;
        }
        char *colon = strchr(line, ':');
        if (!colon) {
            continue;
        }
        *colon = '\0';
        char *name = line;
        char *value = colon + 1;
        while (*value == ' ') {
            value++;
        }
        http_set_header(request, name, value);
        if (strcasecmp(name, "Content-Length") == 0) {
            content_length = strtoul(value, NULL, 10);
        }
    }

    if (content_length > 0 && content_length < sizeof(request->body) && body_start) {
        while (*body_start == '\r' || *body_start == '\n') {
            body_start++;
        }
        size_t remaining = strlen(body_start);
        if (remaining > 0) {
            size_t copy_len = remaining < content_length ? remaining : content_length;
            memcpy(request->body, body_start, copy_len);
            request->body[copy_len] = '\0';
            request->body_length = copy_len;
        }
    }

    return 0;
}

void http_prepare_response(struct HttpResponse *response, int status_code, const char *reason_phrase, const char *content_type, const char *body)
{
    if (!response) {
        return;
    }

    memset(response, 0, sizeof(*response));
    response->status_code = status_code;
    snprintf(response->reason_phrase, sizeof(response->reason_phrase), "%s", reason_phrase ? reason_phrase : "OK");
    if (content_type && *content_type) {
        snprintf(response->headers[response->header_count].name, sizeof(response->headers[response->header_count].name), "Content-Type");
        snprintf(response->headers[response->header_count].value, sizeof(response->headers[response->header_count].value), "%s", content_type);
        response->header_count++;
    }
    if (body) {
        snprintf(response->body, sizeof(response->body), "%s", body);
        response->body_length = strlen(response->body);
    }
}

const char *http_mime_for_path(const char *path)
{
    if (!path) {
        return "application/octet-stream";
    }

    const char *ext = strrchr(path, '.');
    if (!ext) {
        return "application/octet-stream";
    }

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
        return "text/html";
    }
    if (strcmp(ext, ".json") == 0) {
        return "application/json";
    }
    if (strcmp(ext, ".txt") == 0) {
        return "text/plain";
    }
    if (strcmp(ext, ".css") == 0) {
        return "text/css";
    }
    if (strcmp(ext, ".js") == 0) {
        return "application/javascript";
    }
    if (strcmp(ext, ".png") == 0) {
        return "image/png";
    }
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        return "image/jpeg";
    }
    return "application/octet-stream";
}
