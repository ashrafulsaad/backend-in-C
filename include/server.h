#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server;

#define MAX_HEADERS 32
#define MAX_ROUTES 32
#define MAX_MIDDLEWARE 8
#define MAX_BODY_SIZE 65536
#define DEFAULT_PORT 8080
#define DEFAULT_BACKLOG 128
#define DEFAULT_THREAD_COUNT 4

struct HttpHeader {
    char name[64];
    char value[256];
};

struct HttpRequest {
    char method[16];
    char path[256];
    char query[256];
    char version[16];
    struct HttpHeader headers[MAX_HEADERS];
    size_t header_count;
    char body[MAX_BODY_SIZE];
    size_t body_length;
};

struct HttpResponse {
    int status_code;
    char reason_phrase[64];
    struct HttpHeader headers[MAX_HEADERS];
    size_t header_count;
    char body[MAX_BODY_SIZE];
    size_t body_length;
};

typedef void (*HttpHandler)(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);
typedef void (*Middleware)(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, HttpHandler next, void *context);

struct Route {
    char method[16];
    char path[256];
    HttpHandler handler;
    void *context;
};

struct Router {
    struct Route routes[MAX_ROUTES];
    size_t route_count;
};

struct ThreadPool {
    pthread_t *threads;
    size_t thread_count;
    int *jobs;
    size_t queue_size;
    size_t head;
    size_t tail;
    size_t count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool stop;
};

struct AppConfig {
    int port;
    int backlog;
    int thread_count;
    char host[64];
    char log_file[128];
    bool enable_db;
    char db_host[128];
    int db_port;
    char db_name[128];
    char db_user[128];
    char db_password[128];
};

struct DatabaseConnection {
    bool configured;
    char host[128];
    int port;
    char name[128];
    char user[128];
    char password[128];
};

struct Server {
    int domain;
    int service;
    int protocol;
    unsigned long netinterface;
    int port;
    int backlog;
    struct sockaddr_in address;
    int socket;
    int epoll_fd;
    struct Router router;
    struct ThreadPool thread_pool;
    struct AppConfig config;
    struct DatabaseConnection db;
    Middleware middlewares[MAX_MIDDLEWARE];
    void *middleware_contexts[MAX_MIDDLEWARE];
    size_t middleware_count;
    void (*launch)(struct Server *server);
};

struct Server *server_constructor(
    int domain,
    int service,
    int protocol,
    unsigned long netinterface,
    int port,
    int backlog,
    void (*launch)(struct Server *server)
);

void server_destroy(struct Server *server);
void server_launch(struct Server *server);
void server_register_default_routes(struct Server *server);
void server_add_middleware(struct Server *server, Middleware middleware, void *context);
void server_log(const char *level, const char *fmt, ...);
void server_register_route(struct Server *server, const char *method, const char *path, HttpHandler handler, void *context);
int server_initialize_database(struct Server *server);
void server_run_benchmark(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);
void server_finalize_request(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);

int socket_create_listener(int domain, int service, int protocol, unsigned long netinterface, int port, int backlog);
void socket_close(int fd);
void server_send_response(int client_fd, struct HttpResponse *response);

int http_parse_request(const char *input, struct HttpRequest *request);
void http_prepare_response(struct HttpResponse *response, int status_code, const char *reason_phrase, const char *content_type, const char *body);
const char *http_mime_for_path(const char *path);

void router_register(struct Server *server, const char *method, const char *path, HttpHandler handler, void *context);
void router_dispatch(struct Server *server, struct HttpRequest *request, struct HttpResponse *response);

void middleware_add(struct Server *server, Middleware middleware, void *context);
void middleware_apply(struct Server *server, size_t index, struct HttpRequest *request, struct HttpResponse *response, HttpHandler next);
void middleware_security(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, HttpHandler next, void *context);
void middleware_logging(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, HttpHandler next, void *context);

void threadpool_init(struct Server *server);
void threadpool_destroy(struct Server *server);
void threadpool_enqueue(struct Server *server, int client_fd);

void route_home(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);
void route_health(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);
void route_db(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);
void route_benchmark(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);
void route_echo(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);
void route_not_found(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context);

#endif
