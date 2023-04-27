#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int num_commands = argc - 1;
    int pipe_commands[num_commands - 1][2]; // array of pipes

    // error if no commands provided
    if (num_commands == 0) {
        fprintf(stderr, "error: no arguments provided\n");
        exit(EINVAL);
    }

    // create pipes between adjacent commands
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipe_commands[i]) == -1) {
            perror("pipe");
            exit(errno);
        }
    }

    // execute each command in a new process
    for (int i = 0; i < num_commands; i++) {
        int pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(errno);
        } else if (pid == 0) { // child process
            // set up input redirection from previous command
            if (i > 0) {
                if (dup2(pipe_commands[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(errno);
                }
            }

            // set up output redirection to next command
            if (i < num_commands - 1) {
                if (dup2(pipe_commands[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(errno);
                }
            }

            // close all pipes
            for (int j = 0; j < num_commands - 1; j++) {
                if (close(pipe_commands[j][0]) == -1) {
                    perror("close");
                    exit(errno);
                }
                if (close(pipe_commands[j][1]) == -1) {
                    perror("close");
                    exit(errno);
                }
            }

            // execute command
            execlp(argv[i + 1], argv[i + 1], NULL);

            // if execlp returns, there was an error
            perror("execlp");
            exit(errno);
        }
    }

    // close all pipes in parent process
    for (int i = 0; i < num_commands - 1; i++) {
        if (close(pipe_commands[i][0]) == -1) {
            perror("close");
            exit(errno);
        }
        if (close(pipe_commands[i][1]) == -1) {
            perror("close");
            exit(errno);
        }
    }

    // wait for all child processes to finish
    int status;
    for (int i = 0; i < num_commands; i++) {
        if (wait(&status) == -1) {
            perror("wait");
            exit(errno);
        }
    }

    return 0;
}
