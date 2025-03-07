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

    char *exe = find_exe(args[0]);

    if (exe == NULL) {
        printf("cannot find command: %s\n", args[0]);
        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        printf("execution failed, cannot fork. errno: %d\n", errno);
    } else if (pid == 0) {
        for (int i = 0; i < arg_len; i++) {
            if (strcmp(args[i], REDIR) != 0) {
                continue;
            }

            if (args[i + 1] == NULL) {
                printf("provide a file for redirection\n");
                exit(1);
            }

            redirect(args[i + 1]);
            args[i] = NULL;
        }

        int e = execv(exe, args);
        if (e == -1) {
            printf("execution failed. errno: %d\n", errno);
            exit(1);
        }
    } else {
        wait(NULL);
    }
}
