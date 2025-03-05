#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH_CNT 100
#define PATH_FILE_NAME ".wishpath"

static char *get_path_file_path() {
    char *home = getenv("HOME");

    size_t len = strlen(home) + strlen(PATH_FILE_NAME) + 2;
    char *path_file = (char *)malloc(len * sizeof(char));

    strcpy(path_file, home);
    strcat(path_file, "/");
    strcat(path_file, PATH_FILE_NAME);

    return path_file;
}

int add_path(char *new_path) {
    char *path = (char *)malloc((strlen(new_path) + 2) * sizeof(char));
    strcpy(path, new_path);
    strcat(path, "\n");

    char *path_file_path = get_path_file_path();

    FILE *path_file = fopen(path_file_path, "a");
    if (path_file == NULL) {
        printf("cannot open %s (err: %s)\n", path_file_path, strerror(errno));
        return -1;
    }

    size_t len = strlen(path);
    size_t written = fwrite(path, sizeof(char), len, path_file);

    if (len != written) {
        printf("cannot write to %s\n", path_file_path);
        return -1;
    }

    if (fclose(path_file) == EOF) {
        printf("cannot write to %s (err: %s)\n", path_file_path,
               strerror(errno));
        return -1;
    }

    return 0;
}

char **get_paths() {
    char **paths = (char **)calloc(MAX_PATH_CNT, sizeof(char *));
    int i = 0;

    char *path_file_path = get_path_file_path();
    FILE *fp = fopen(path_file_path, "r");

    while (i < MAX_PATH_CNT) {
        size_t cap = 0;
        ssize_t l = getline(&paths[i], &cap, fp);
        size_t len = strlen(paths[i]);

        paths[i][len - 1] = '\0';

        if (l == -1) {
            if (errno == ENOMEM) {
                printf("error reading %s. (err: %s)\n", path_file_path,
                       strerror(errno));

                for (int j = 0; j < i; ++j) {
                    free(paths[j]);
                }
                free(paths);

                return NULL;
            }

            break;
        }

        i++;
    }

    return paths;
}

char *find_exe(char *name) {
    if (name[0] == '\0') {
        return NULL;
    } else if (name[0] == '/') {
        return name;
    }

    size_t name_len = strlen(name);
    char **paths = get_paths();

    for (int i = 0; paths[i] != NULL; i++) {
        size_t len = strlen(paths[i]);

        if (len == 0) {
            continue;
        }

        char *exe_path = (char *)malloc(name_len + len + 2);

        strcpy(exe_path, paths[i]);
        strcat(exe_path, "/");
        strcat(exe_path, name);

        if (access(exe_path, X_OK) == 0) {
            return exe_path;
        }
    }

    return NULL;
}
