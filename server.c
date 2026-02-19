//
// Created by kiana on 2/19/26.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    //create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (server_fd < 0) {
        perror("socket failed");
        exit(1);
    }

    // set address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(1);
    }

    // listen
    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(1);
    }
    printf("Server listening on port %d...\n", PORT);

    // accept
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_fd < 0) {
        perror("accept failed");
        exit(1);
    }
    printf("Client connected.\n");
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        read(client_fd, buffer, BUFFER_SIZE);

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        int pipefd[2];
        pipe(pipefd);

        pid_t pid = fork();

        if (pid == 0) {
            // Child

            close(pipefd[0]); // close read end

            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);

            close(pipefd[1]);

            // Remove newline
            buffer[strcspn(buffer, "\n")] = 0;

            char *args[] = {"/bin/sh", "-c", buffer, NULL};
            execv("/bin/sh", args);

            perror("exec failed");
            exit(1);
        }
        else {
            // Parent
            close(pipefd[1]); // close write end

            ssize_t bytes_read;

            while ((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE)) > 0) {
                write(client_fd, buffer, bytes_read);
            }

            close(pipefd[0]);
            wait(NULL);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}