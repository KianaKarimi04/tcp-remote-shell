#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define POOL_SIZE 4
#define MAX_ARGS 100

void handle_client(int client_fd);

int main() {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    signal(SIGCHLD, SIG_IGN);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    // Create process pool
    for (int i = 0; i < POOL_SIZE; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            while (1) {
                int client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
                if (client_fd < 0) {
                    perror("accept failed");
                    continue;
                }

                handle_client(client_fd);
                close(client_fd);
            }
            exit(0);
        }
    }

    // Parent waits
    while (1)
        pause();

    close(server_fd);
    return 0;
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t n = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (n <= 0)
            break;

        buffer[n] = '\0';

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        // Remove newline
        buffer[strcspn(buffer, "\n")] = 0;

        // Parse arguments
        char *args[MAX_ARGS];
        char *input_file = NULL;
        char *output_file = NULL;
        char *error_file = NULL;

        int arg_count = 0;

        char *token = strtok(buffer, " ");
        while (token != NULL) {

            if (strcmp(token, "<") == 0) {
                token = strtok(NULL, " ");
                input_file = token;
            }
            else if (strcmp(token, ">") == 0) {
                token = strtok(NULL, " ");
                output_file = token;
            }
            else if (strcmp(token, "2>") == 0) {
                token = strtok(NULL, " ");
                error_file = token;
            }
            else {
                if (arg_count < MAX_ARGS - 1)
                    args[arg_count++] = token;
            }

            token = strtok(NULL, " ");
        }

        args[arg_count] = NULL;

        if (args[0] == NULL)
            continue;

        int pipefd[2];
        if (pipe(pipefd) < 0) {
            perror("pipe failed");
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            close(pipefd[0]);

            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);

            if (input_file) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) {
					perror("Open failed");
					exit(1);
                }
				dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (output_file) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd >= 0) {
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            }

            if (error_file) {
                int fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd >= 0) {
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                }
            }

            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        }
        else {
            close(pipefd[1]);

            ssize_t bytes_read;
            while ((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE)) > 0) {
                write(client_fd, buffer, bytes_read);
            }

            close(pipefd[0]);
            wait(NULL);

            char *end_marker = "__END__\n";
            write(client_fd, end_marker, strlen(end_marker));
        }
    }
}