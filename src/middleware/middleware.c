#include "server.h"

#include <string.h>

struct MiddlewareChainState {
    struct Server *server;
    struct HttpRequest *request;
    struct HttpResponse *response;
    size_t index;
    HttpHandler final_handler;
};

static void middleware_continue_chain(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)server;
    (void)request;
    (void)response;

    struct MiddlewareChainState *state = (struct MiddlewareChainState *)context;
    if (!state) {
        return;
    }

    middleware_apply(state->server, state->index, state->request, state->response, state->final_handler);
}

void middleware_add(struct Server *server, Middleware middleware, void *context)
{
    if (!server || server->middleware_count >= MAX_MIDDLEWARE) {
        return;
    }

    server->middlewares[server->middleware_count] = middleware;
    server->middleware_contexts[server->middleware_count] = context;
    server->middleware_count++;
}

void middleware_apply(struct Server *server, size_t index, struct HttpRequest *request, struct HttpResponse *response, HttpHandler next)
{
    if (!server || index >= server->middleware_count) {
        if (next) {
            next(server, request, response, NULL);
        }
        return;
    }

    struct MiddlewareChainState state = {
        .server = server,
        .request = request,
        .response = response,
        .index = index + 1,
        .final_handler = next,
    };

    server->middlewares[index](server, request, response, middleware_continue_chain, &state);
}

void middleware_security(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, HttpHandler next, void *context)
{
    (void)server;
    (void)context;
    if (strstr(request->path, "..")) {
        http_prepare_response(response, 400, "Bad Request", "text/plain", "Path traversal is not allowed");
        return;
    }
    next(server, request, response, context);
}

void middleware_logging(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, HttpHandler next, void *context)
{
    (void)response;
    server_log("INFO", "Request %s %s", request->method, request->path);
    next(server, request, response, context);
}
