#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define DEFAULT_PREFIX "x"
#define DEFAULT_LINE 1000
#define DEFAULT_NUM_SUFFIX 2

char *default_suffix = "abcdefghijklmnopqrstuvwxyz";

int checkInt(char *arg) {
  for (int n = 0; arg[n]; n++) {
    if (arg[n] < '0' || arg[n] > '9') return 0;
  }
  return 1;
}

int nextSuffix(char *suffix) {
    int carrier = 1;
    int suffix_len = strlen(suffix);
    for (int i = suffix_len-1; i>=0; i--) {
        if (carrier == 1) {
            carrier = 0;
            char *cursor = strchr(default_suffix, *(suffix+i))+1;
            if (cursor-default_suffix == strlen(default_suffix)) {
                cursor = default_suffix;
                carrier = 1;
            }
            *(suffix+i) = *cursor;
        }
    }
    return carrier;
}

void split(int fd, int max_line, int max_suffix, char *prefix) {
    char suffix[max_suffix];
    for (int i = 0; i < max_suffix; i++) {
        suffix[i] = *default_suffix;
    }

    int fd_parts;
    char path_parts[1000];
    strcpy(path_parts, prefix);
    strcpy(path_parts+strlen(prefix), suffix);
    fd_parts = open(path_parts, O_WRONLY | O_CREATE);

    char buf[1];
    int num_line = 0;
    while(read(fd, buf, 1) > 0) {
        write(fd_parts, buf, 1);
        if (*buf == '\n') num_line++;
        if (num_line == max_line) {
            num_line = 0;
            close(fd_parts);
            if (nextSuffix(suffix) == 1) {
                printf(2, "split: not enough suffix\n");
                exit();
            }
            strcpy(path_parts+strlen(prefix), suffix);
            fd_parts = open(path_parts, O_WRONLY | O_CREATE);
        }
    }
}

int main(int argc, char *argv[]) {
    char prefix[1000];
    int fd;
    int max_line = DEFAULT_LINE;
    int max_suffix = DEFAULT_NUM_SUFFIX;
    strcpy(prefix, DEFAULT_PREFIX);
    int pos=argc;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            if (i+1 >= argc) {
                printf(2, "split: wrong arguments\n");
                exit();
            }
            if (checkInt(argv[i+1]) == 0) {
                printf(2, "split: wrong arguments\n");
                exit();
            }

            max_suffix = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-l") == 0) {
            if (i+1 >= argc) {
                printf(2, "split: wrong arguments\n");
                exit();
            }
            if (checkInt(argv[i+1]) == 0) {
                printf(2, "split: wrong arguments\n");
                exit();
            }

            max_line = atoi(argv[++i]);
        } else {
            pos = i;
            break;
        }
    }

    if (pos+1 < argc) {
        memset(prefix, 0, 1000);
        strcpy(prefix, argv[pos+1]);
    }

    fd = open(argv[pos], O_RDONLY);
    if (fd < 0) {
        printf(2, "split: cannot open\n", argv[pos]);
        exit();
    } else {
        struct stat st;
        if (fstat(fd, &st) < 0) {
            printf(2, "split: cannot stat\n", argv[pos]);
            close(fd);
            exit();
        }
        if (st.type != T_FILE) {
            printf(2, "split: cannot split\n", argv[pos]);
            close(fd);
            exit();
        }
        split(fd, max_line, max_suffix, prefix);

        // printf(1, "max_line: %d\n", max_line);
        // printf(1, "max_suffix: %d\n", max_suffix);
        // printf(1, "prefix: %s\n", prefix);
        // printf(1, "path: %s\n", argv[pos]);
        close(fd);
        exit();
    }
    
    exit();
}
