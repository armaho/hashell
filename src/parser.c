#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include "path.h"

#define MAX_ARGS 100
#define EXIT_CMD "exit"
#define PATH_CMD "path"
#define CD_CMD "cd"
#define REDIR ">"
#define PIPE "|"

int extract_args(char ***args, size_t *arg_len, char *command) {
    *args = malloc(MAX_ARGS * sizeof(char *));
    if (args == NULL) {
        printf("cannot extract args, memory allocation failed. errno: %d\n",
               errno);
        return -1;
    }

    char *arg;
    size_t i = 0;

    while ((arg = strsep(&command, " ")) != NULL) {
        if (MAX_ARGS - 1 <= i) {
            printf("cannot extract args, too many arguments.\n");
            return -1;
        }
        (*args)[i] = arg;
        i++;
    }
    (*args)[i] = NULL;

    *arg_len = i;
    return 0;
}

void redirect(char *file) {
    close(STDOUT_FILENO);
    if (open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU) == -1) {
        printf("cannot open '%s'. (err: %s)\n", file, strerror(errno));
        exit(1);
    }
}

int extract_commands(char ****cmds, size_t *cmd_len, char **args) {
    *cmds = malloc(MAX_ARGS * sizeof(char **));
    if (args == NULL) {
        printf("cannot extract commands, memory allocation failed. (err: %s)\n",
               strerror(errno));
        return -1;
    }

    size_t c_len = 0;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], PIPE) != 0) {
            continue;
        }

        args[i] = NULL;

        (*cmds)[c_len] = args;
        c_len++;

        args = &args[i + 1];
        i = 0;
    }

    if (args[0] != NULL) {
        (*cmds)[c_len] = args;
        c_len++;
    }

    *cmd_len = c_len;
    return 0;
}

void run(char **args, int input_fd, int output_fd, int redirection_allowed) {
    if (input_fd != STDIN_FILENO) {
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            printf("dup2 input faild.\n");
            exit(1);
        }
        close(input_fd);
    }

    if (output_fd != STDOUT_FILENO) {
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            printf("dup2 output faild.\n");
            exit(1);
        }
        close(output_fd);
    }

    char *exe = find_exe(args[0]);

    if (exe == NULL) {
        printf("cannot find command: %s.\n", args[0]);
        return;
    }

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], REDIR) != 0) {
            continue;
        }

        if (!redirection_allowed) {
            printf("cannot redirect the output.\n");
            exit(1);
        }

        if (args[i + 1] == NULL) {
            printf("provide a file for redirection.\n");
            exit(1);
        }

        redirect(args[i + 1]);
        args[i] = NULL;
    }

    execv(exe, args);
}

void parse(char *command) {
    char **args = NULL;
    size_t arg_len = 0;

    if (extract_args(&args, &arg_len, command) == -1) {
        printf("execution failed, cannot read args.\n");
        return;
    }

    if (args[0] == NULL) {
        return;
    }

    if (strcmp(args[0], EXIT_CMD) == 0) {
        exit(0);
    }

    if (strcmp(args[0], PATH_CMD) == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            if (add_path(args[i]) == -1) {
                printf("cannot add '%s' to path\n", args[i]);
            }
        }
        return;
    }

    if (strcmp(args[0], CD_CMD) == 0) {
        int cd = chdir(args[1]);
        if (cd == -1) {
            printf("cannot change directory. (err: %s)\n", strerror(errno));
        }

        return;
    }

    char ***cmds = NULL;
    size_t c_cnt = 0;
    if (extract_commands(&cmds, &c_cnt, args) == -1) {
        printf("cannot extract commands.\n");
        return;
    }

    int in_fileno = STDIN_FILENO;

    for (int i = 0; i < c_cnt; i++) {
        int last_cmd = (i == c_cnt - 1);
        int pipe_fd[2];

        if (!last_cmd) {
            if (pipe(pipe_fd) == -1) {
                printf("pipe failed.\n");
                return;
            }
        } else {
            pipe_fd[1] = STDOUT_FILENO;
        }

        pid_t pid = fork();

        if (pid == -1) {
            printf("cannot fork.\n");
            return;
        } else if (pid == 0) {
            run(cmds[i], in_fileno, pipe_fd[1], last_cmd);

            printf("execution failed. errno: %d\n", errno);
            return;
        } else {
            if (in_fileno != STDIN_FILENO) {
                close(in_fileno);
            }

            if (pipe_fd[1] != STDOUT_FILENO) {
                close(pipe_fd[1]);
            }
        }

        if (!last_cmd) {
            in_fileno = pipe_fd[0];
        }
    }

    while (wait(NULL) != -1) {
    }
}
