#if defined(__linux__)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#elif defined(_WIN32)

#include <winsock.h>

#endif

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void assert_error(int err)
{
#if defined(__linux__)
    if(err < 0) {
        fprintf(stderr, "[OS Error %d] ", errno);
        perror("");
        abort();
    }
#elif defined(_WIN32)
    if(err < 0) {
        DWORD dw = GetLastError();
        fprintf(stderr, "[OS Error %d]\n", dw);
        abort();
    }
#endif
}

int main(int argc, char* argv[])
{
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(0x0202, &wsaData);
#endif

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    assert_error(client_socket);
    int command = 0;
    int quote = 0;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(16600);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "--host") && i + 1 < argc) {
            addr.sin_addr.s_addr = inet_addr(argv[i + 1]);
            i += 1;
        }
        else if(!strcmp(argv[i], "--port") && i + 1 < argc) {
            addr.sin_port = htons(atoi(argv[i + 1]));
            i += 1;
        } else if(!strcmp(argv[i], "--quote")) {
            quote = 1;
        } else {
            command = i;
            break;
        }
    }

    if(!command) {
        printf("USAGE:\n    %s [--host <ip>] "
                "[--port <port>] <command>\n", argv[0]);
        exit(255);
    }

    assert_error(connect(client_socket,
            (struct sockaddr*)&addr, sizeof(addr)));

    // concat arguments
    uint32_t size = 0;
    for(int i = command; i < argc; i++)
        size += strlen(argv[i]) + 1;
    char* buf = (char*) malloc(size * 2);
    char* pstr = buf;
    for(int i = command; i < argc; i++) {
        char* cur_arg = argv[i];

        if(quote)
            *(pstr++) = '\'';

        for(; *cur_arg != '\0'; cur_arg++) {
            if(quote && *cur_arg == '\'') {
                pstr[0] = '\'';
                pstr[1] = '\"';
                pstr[2] = '\'';
                pstr[3] = '\"';
                pstr[4] = '\'';
                pstr += 5;
            } else
                *(pstr++) = *cur_arg;
        }

        if(quote)
            *(pstr++) = '\'';

        *(pstr++) = i < argc - 1 ? ' ' : '\0';
    }

    size = pstr - buf;

    assert_error(send(client_socket, (char*) &size, sizeof(size), 0));
    assert_error(send(client_socket, buf, size, 0));

    free(buf);

    close(client_socket);

#if defined(_WIN32)
    WSACleanup();
#endif
}
