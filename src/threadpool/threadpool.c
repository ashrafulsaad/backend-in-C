#include "server.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void *threadpool_worker(void *arg)
{
    struct Server *server = (struct Server *)arg;
    while (1) {
        pthread_mutex_lock(&server->thread_pool.mutex);
        while (!server->thread_pool.stop && server->thread_pool.count == 0) {
            pthread_cond_wait(&server->thread_pool.cond, &server->thread_pool.mutex);
        }
        if (server->thread_pool.stop && server->thread_pool.count == 0) {
            pthread_mutex_unlock(&server->thread_pool.mutex);
            break;
        }

        int client_fd = server->thread_pool.jobs[server->thread_pool.head];
        server->thread_pool.head = (server->thread_pool.head + 1) % server->thread_pool.queue_size;
        server->thread_pool.count--;
        pthread_mutex_unlock(&server->thread_pool.mutex);

        if (client_fd >= 0) {
            char buffer[MAX_BODY_SIZE];
            ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                struct HttpRequest request;
                memset(&request, 0, sizeof(request));
                if (http_parse_request(buffer, &request) == 0) {
                    struct HttpResponse response;
                    memset(&response, 0, sizeof(response));
                    middleware_apply(server, 0, &request, &response, server_finalize_request);
                    server_send_response(client_fd, &response);
                } else {
                    struct HttpResponse response;
                    memset(&response, 0, sizeof(response));
                    http_prepare_response(&response, 400, "Bad Request", "text/plain", "Malformed HTTP request");
                    server_send_response(client_fd, &response);
                }
            }
            close(client_fd);
        }
    }
    return NULL;
}

void threadpool_init(struct Server *server)
{
    if (!server) {
        return;
    }

    server->thread_pool.thread_count = server->config.thread_count > 0 ? server->config.thread_count : DEFAULT_THREAD_COUNT;
    server->thread_pool.queue_size = 256;
    server->thread_pool.jobs = calloc(server->thread_pool.queue_size, sizeof(int));
    server->thread_pool.threads = calloc(server->thread_pool.thread_count, sizeof(pthread_t));
    pthread_mutex_init(&server->thread_pool.mutex, NULL);
    pthread_cond_init(&server->thread_pool.cond, NULL);
    server->thread_pool.stop = false;

    for (size_t i = 0; i < server->thread_pool.thread_count; ++i) {
        pthread_create(&server->thread_pool.threads[i], NULL, threadpool_worker, server);
    }
}

void threadpool_destroy(struct Server *server)
{
    if (!server) {
        return;
    }

    server->thread_pool.stop = true;
    pthread_cond_broadcast(&server->thread_pool.cond);
    for (size_t i = 0; i < server->thread_pool.thread_count; ++i) {
        if (server->thread_pool.threads[i]) {
            pthread_join(server->thread_pool.threads[i], NULL);
        }
    }

    pthread_mutex_destroy(&server->thread_pool.mutex);
    pthread_cond_destroy(&server->thread_pool.cond);
    free(server->thread_pool.jobs);
    free(server->thread_pool.threads);
}

void threadpool_enqueue(struct Server *server, int client_fd)
{
    if (!server) {
        return;
    }

    pthread_mutex_lock(&server->thread_pool.mutex);
    if (server->thread_pool.count >= server->thread_pool.queue_size) {
        pthread_mutex_unlock(&server->thread_pool.mutex);
        close(client_fd);
        return;
    }

    server->thread_pool.jobs[server->thread_pool.tail] = client_fd;
    server->thread_pool.tail = (server->thread_pool.tail + 1) % server->thread_pool.queue_size;
    server->thread_pool.count++;
    pthread_cond_signal(&server->thread_pool.cond);
    pthread_mutex_unlock(&server->thread_pool.mutex);
}
