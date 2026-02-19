//
// Created by kiana on 2/19/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        return 1;
    }

    while (1) {
        printf("shell> ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
            break;

        write(sock, buffer, strlen(buffer));

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        memset(buffer, 0, BUFFER_SIZE);

        while (1) {
            ssize_t n = read(sock, buffer, BUFFER_SIZE - 1);
            if (n <= 0)
                break;

            buffer[n] = '\0';

            char *end_pos = strstr(buffer, "__END__");
            if (end_pos != NULL) {
                *end_pos = '\0';   // cut marker out
                printf("%s", buffer);
                break;
            }

            printf("%s", buffer);
        }
    }

    close(sock);
    return 0;
}
