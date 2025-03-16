#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "parser.h"

int main(int argc, char* argv[argc + 1]) {
    char cwd[PATH_MAX];

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            printf("getcwd() error\n");
        }

        printf("HaShell %s> ", cwd);

        char* command = NULL;
        size_t command_capp = 0;

        if (getline(&command, &command_capp, stdin) == -1) {
            printf("error reading command\n");
        }

        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        parse(command);
    }

    return 0;
}
