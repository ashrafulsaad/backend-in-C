#include "server.h"
#include "http/http.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void server_set_status(struct HttpResponse *response, int status_code, const char *reason_phrase)
{
    response->status_code = status_code;
    snprintf(response->reason_phrase, sizeof(response->reason_phrase), "%s", reason_phrase);
}

static void server_add_header(struct HttpResponse *response, const char *name, const char *value)
{
    if (response->header_count >= MAX_HEADERS) {
        return;
    }
    snprintf(response->headers[response->header_count].name, sizeof(response->headers[response->header_count].name), "%s", name);
    snprintf(response->headers[response->header_count].value, sizeof(response->headers[response->header_count].value), "%s", value);
    response->header_count++;
}

void server_log(const char *level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "[%s] ", level);
    vfprintf(stdout, fmt, args);
    fputc('\n', stdout);
    va_end(args);
}

struct Server *server_constructor(int domain, int service, int protocol, unsigned long netinterface, int port, int backlog, void (*launch)(struct Server *server))
{
    struct Server *server = calloc(1, sizeof(*server));
    if (!server) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    server->domain = domain;
    server->service = service;
    server->protocol = protocol;
    server->netinterface = netinterface;
    server->port = port > 0 ? port : DEFAULT_PORT;
    server->backlog = backlog > 0 ? backlog : DEFAULT_BACKLOG;
    snprintf(server->config.host, sizeof(server->config.host), "0.0.0.0");
    server->config.port = server->port;
    server->config.backlog = server->backlog;
    server->config.thread_count = DEFAULT_THREAD_COUNT;
    server->config.enable_db = false;
    snprintf(server->config.log_file, sizeof(server->config.log_file), "server.log");

    const char *env_port = getenv("BACKEND_PORT");
    if (env_port && *env_port) {
        server->config.port = atoi(env_port);
    }
    const char *env_threads = getenv("BACKEND_THREADS");
    if (env_threads && *env_threads) {
        server->config.thread_count = atoi(env_threads);
    }
    const char *env_db_host = getenv("DB_HOST");
    if (env_db_host && *env_db_host) {
        server->config.enable_db = true;
        snprintf(server->config.db_host, sizeof(server->config.db_host), "%s", env_db_host);
    }
    const char *env_db_name = getenv("DB_NAME");
    if (env_db_name && *env_db_name) {
        snprintf(server->config.db_name, sizeof(server->config.db_name), "%s", env_db_name);
    }
    const char *env_db_user = getenv("DB_USER");
    if (env_db_user && *env_db_user) {
        snprintf(server->config.db_user, sizeof(server->config.db_user), "%s", env_db_user);
    }
    const char *env_db_password = getenv("DB_PASSWORD");
    if (env_db_password && *env_db_password) {
        snprintf(server->config.db_password, sizeof(server->config.db_password), "%s", env_db_password);
    }

    server->socket = socket_create_listener(domain, service, protocol, netinterface, server->config.port, server->config.backlog);
    if (server->socket < 0) {
        exit(EXIT_FAILURE);
    }

    server->epoll_fd = epoll_create1(0);
    if (server->epoll_fd < 0) {
        perror("epoll_create1");
        socket_close(server->socket);
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN;
    event.data.fd = server->socket;
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->socket, &event) < 0) {
        perror("epoll_ctl");
        close(server->epoll_fd);
        socket_close(server->socket);
        exit(EXIT_FAILURE);
    }

    threadpool_init(server);
    server->launch = launch;
    server_register_default_routes(server);
    server_add_middleware(server, middleware_security, NULL);
    server_add_middleware(server, middleware_logging, NULL);
    server_initialize_database(server);
    server_log("INFO", "Server listening on port %d with %zu worker threads", server->config.port, server->thread_pool.thread_count);
    return server;
}

void server_destroy(struct Server *server)
{
    if (!server) {
        return;
    }

    threadpool_destroy(server);
    close(server->epoll_fd);
    socket_close(server->socket);
    free(server);
}

void server_launch(struct Server *server)
{
    if (!server) {
        return;
    }

    struct epoll_event events[32];
    while (1) {
        int event_count = epoll_wait(server->epoll_fd, events, 32, -1);
        if (event_count < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < event_count; ++i) {
            if (events[i].data.fd == server->socket) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server->socket, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }
                threadpool_enqueue(server, client_fd);
            }
        }
    }
}

void server_register_default_routes(struct Server *server)
{
    server_register_route(server, "GET", "/", route_home, NULL);
    server_register_route(server, "GET", "/health", route_health, NULL);
    server_register_route(server, "GET", "/db", route_db, NULL);
    server_register_route(server, "GET", "/benchmark", route_benchmark, NULL);
    server_register_route(server, "POST", "/api/echo", route_echo, NULL);
    server_register_route(server, "*", "/", route_not_found, NULL);
}

void server_add_middleware(struct Server *server, Middleware middleware, void *context)
{
    middleware_add(server, middleware, context);
}

void server_register_route(struct Server *server, const char *method, const char *path, HttpHandler handler, void *context)
{
    router_register(server, method, path, handler, context);
}

int server_initialize_database(struct Server *server)
{
    if (!server || !server->config.enable_db) {
        server_log("INFO", "Database integration is disabled; using stubbed adapter");
        return 0;
    }

    snprintf(server->db.host, sizeof(server->db.host), "%s", server->config.db_host);
    server->db.port = server->config.db_port > 0 ? server->config.db_port : 5432;
    snprintf(server->db.name, sizeof(server->db.name), "%s", server->config.db_name);
    snprintf(server->db.user, sizeof(server->db.user), "%s", server->config.db_user);
    snprintf(server->db.password, sizeof(server->db.password), "%s", server->config.db_password);
    server->db.configured = true;
    server_log("INFO", "Database adapter ready for %s@%s:%d", server->db.user, server->db.host, server->db.port);
    return 1;
}

void server_run_benchmark(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)server;
    (void)request;
    (void)context;
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < 2000; ++i) {
        (void)i;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    char body[256];
    snprintf(body, sizeof(body), "benchmark_complete elapsed_seconds=%.6f", elapsed);
    server_set_status(response, 200, "OK");
    server_add_header(response, "Content-Type", "text/plain");
    snprintf(response->body, sizeof(response->body), "%s", body);
    response->body_length = strlen(response->body);
}

void server_finalize_request(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)context;
    router_dispatch(server, request, response);
}

void route_home(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)server;
    (void)context;
    server_set_status(response, 200, "OK");
    server_add_header(response, "Content-Type", "text/html");
    snprintf(response->body, sizeof(response->body),
             "<html><body><h1>Reusable C HTTP Server</h1>"
             "<p>Method: %s</p><p>Path: %s</p><p>Query: %s</p>"
             "<p>Routes: /health, /db, /benchmark, /api/echo</p></body></html>",
             request->method, request->path, request->query[0] ? request->query : "(none)");
    response->body_length = strlen(response->body);
}

void route_health(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)server;
    (void)context;
    server_set_status(response, 200, "OK");
    server_add_header(response, "Content-Type", "application/json");
    snprintf(response->body, sizeof(response->body),
             "{\"status\":\"ok\",\"method\":\"%s\",\"path\":\"%s\",\"query\":\"%s\"}",
             request->method, request->path, request->query[0] ? request->query : "");
    response->body_length = strlen(response->body);
}

void route_db(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)request;
    (void)context;
    server_set_status(response, 200, "OK");
    server_add_header(response, "Content-Type", "application/json");
    if (server->db.configured) {
        snprintf(response->body, sizeof(response->body), "{\"db\":\"connected\",\"host\":\"%s\",\"status\":\"ready\"}", server->db.host);
    } else {
        snprintf(response->body, sizeof(response->body), "{\"db\":\"stubbed\",\"status\":\"demo-mode\"}");
    }
    response->body_length = strlen(response->body);
}

void route_benchmark(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)request;
    (void)context;
    server_run_benchmark(server, request, response, context);
}

void route_echo(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)server;
    (void)context;
    server_set_status(response, 200, "OK");
    server_add_header(response, "Content-Type", "application/json");
    snprintf(response->body, sizeof(response->body),
             "{\"received\":\"%s\",\"body\":\"%s\",\"query\":\"%s\"}",
             request->method, request->body_length > 0 ? request->body : "", request->query[0] ? request->query : "");
    response->body_length = strlen(response->body);
}

void route_not_found(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    (void)server;
    (void)request;
    (void)context;
    server_set_status(response, 404, "Not Found");
    server_add_header(response, "Content-Type", "text/plain");
    snprintf(response->body, sizeof(response->body), "Route not found");
    response->body_length = strlen(response->body);
}
