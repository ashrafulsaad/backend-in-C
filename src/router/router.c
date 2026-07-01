#include "server.h"

#include <string.h>

void router_register(struct Server *server, const char *method, const char *path, HttpHandler handler, void *context)
{
    if (!server || !handler || server->router.route_count >= MAX_ROUTES) {
        return;
    }

    struct Route *route = &server->router.routes[server->router.route_count++];
    snprintf(route->method, sizeof(route->method), "%s", method);
    snprintf(route->path, sizeof(route->path), "%s", path);
    route->handler = handler;
    route->context = context;
}

void router_dispatch(struct Server *server, struct HttpRequest *request, struct HttpResponse *response)
{
    if (!server || !request || !response) {
        return;
    }

    for (size_t i = 0; i < server->router.route_count; ++i) {
        struct Route *route = &server->router.routes[i];
        if ((strcmp(route->method, request->method) == 0 || strcmp(route->method, "*") == 0) && strcmp(route->path, request->path) == 0) {
            route->handler(server, request, response, route->context);
            return;
        }
    }

    route_not_found(server, request, response, NULL);
}
