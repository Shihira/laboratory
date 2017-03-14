/*
 * Copyright: Shihira Fung <fengzhiping@hotmail.com>
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CONNECTIONS 10

void assert_error(int e)
{
    if(e < 0) {
        fprintf(stderr, "[OS Error %d] ", errno);
        perror("");
        abort();
    }
}

struct recv_stream {
    size_t full_size;
    size_t recv_size;
    char* buf;
};

struct conn_info {
    struct pollfd* fd;
    struct recv_stream stream;
    struct sockaddr_in addr;
};

size_t ncl = 0;

struct pollfd fds[1 + MAX_CONNECTIONS];
struct conn_info clients[MAX_CONNECTIONS];
struct pollfd* cfds = fds + 1;
//struct recv_stream client_streams[1 + MAX_CONNECTIONS];

int new_client(int sock, const struct sockaddr_in* client_addr)
{
    clients[ncl].stream.buf = NULL;
    clients[ncl].stream.recv_size = 0;
    clients[ncl].stream.full_size = 0;

    clients[ncl].fd = &(cfds[ncl]);
    clients[ncl].fd->fd = sock;
    clients[ncl].fd->events = POLLIN;
    clients[ncl].fd->revents = 0;

    memcpy(&(clients[ncl].addr), client_addr, sizeof(struct sockaddr_in));

    ncl += 1;

    return ncl - 1;
}

int read_client(int i)
{
    struct recv_stream* cur = &clients[i].stream;

    if(!cur->full_size) {
        uint32_t size;
        assert_error(recv(clients[i].fd->fd, &size, sizeof(size), 0));

        cur->full_size = size;
        cur->recv_size = 0;
        cur->buf = (char*) malloc(size + 1);
        cur->buf[size] = 0;
    } else {
        ssize_t recv_size = recv(clients[i].fd->fd,
                cur->buf + cur->recv_size,
                cur->full_size - cur->recv_size, 0);
        assert_error(recv_size);
        cur->recv_size += recv_size;
    }

    return cur->full_size - cur->recv_size;
}

void copy_cilent(int dest, int src)
{
    memcpy(cfds + dest, cfds + src, sizeof(struct pollfd));
    memcpy(clients + dest, clients + src, sizeof(struct conn_info));
    clients[dest].fd = cfds + dest;
}

void remove_client(int idx)
{
    if(idx != ncl - 1)
        copy_cilent(idx, ncl - 1);
    free(clients[idx].stream.buf);

    ncl -= 1;
}

int main(int argc, char* argv[])
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert_error(server_socket);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(16600);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "--host") && i + 1 < argc) {
            addr.sin_addr.s_addr = inet_addr(argv[i + 1]);
            i += 1;
        }
        else if(!strcmp(argv[i], "--port") && i + 1 < argc) {
            addr.sin_port = htons(atoi(argv[i + 1]));
            i += 1;
        }
    }

    assert_error(bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)));
    assert_error(listen(server_socket, 0));

    fds->fd = server_socket;
    fds->events = POLLIN;
    fds->revents = 0;

    while(1) {
        assert_error(poll(fds, ncl + 1, -1));

        if(fds->revents & POLLIN) {
            socklen_t len;
            struct sockaddr_in addr;

            int client_fd = accept(fds->fd, (struct sockaddr*)&addr, &len);
            assert_error(client_fd);
            fds->revents = 0;

            new_client(client_fd, &addr);
        }

        for(size_t i = 0; i < ncl; i++) {
            if(!(clients[i].fd->revents & POLLIN))
                continue;

            if(read_client(i) == 0) {
                if(fork() == 0) {
                    printf("[cmdrunner exec] %s\n", clients[i].stream.buf);

                    assert_error(execl("/usr/bin/bash",
                            "/usr/bin/bash", "-c", clients[i].stream.buf));
                } else {
                    close(clients[i].fd->fd);
                    remove_client(i);

                    i -= 1;
                }
            }
        }
    }
}

