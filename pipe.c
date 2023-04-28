#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int num_commands = argc - 1;
    int pipe_commands[num_commands - 1][2];

    if (num_commands == 0) {
        fprintf(stderr, "error: no arguments provided\n");
        exit(EINVAL);
    }

    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipe_commands[i]) == -1) {
            exit(errno);
        }
    }

    for (int i = 0; i < num_commands; i++) {
        
		int pid = fork();

        if (pid == -1) {
            exit(errno);
        }
		else if (pid == 0) {
            if (i > 0) {
                if (dup2(pipe_commands[i - 1][0], STDIN_FILENO) == -1) {
                    exit(errno);
                }
            }

            if (i < num_commands - 1) {
                if (dup2(pipe_commands[i][1], STDOUT_FILENO) == -1) {
                    exit(errno);
                }
            }

            for (int j = 0; j < num_commands - 1; j++) {
                if (close(pipe_commands[j][0]) == -1) {
                    exit(errno);
                }
                if (close(pipe_commands[j][1]) == -1) {
                    exit(errno);
                }
            }

            if (execlp(argv[i + 1], argv[i + 1], NULL) == -1) {
				exit(errno);
			}
			
		}
    }

    for (int i = 0; i < num_commands - 1; i++) {
        if (close(pipe_commands[i][0]) == -1) {
            exit(errno);
        }
        if (close(pipe_commands[i][1]) == -1) {
            exit(errno);
        }
    }

    int status;
	int exit_status = 0;
    for (int i = 0; i < num_commands; i++) {
        if (wait(&status) == -1) {
            exit(errno);
        }
        if (status != 0) {
            exit_status = 1;
        }
    }
    return exit_status;
}
