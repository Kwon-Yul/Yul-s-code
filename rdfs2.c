#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILES 10000

typedef struct {
    char path[512];
    off_t size;
} FileInfo;

FileInfo files[MAX_FILES];
int file_count = 0;

int compare_files(const char *path1, const char *path2) {
    FILE *file1 = fopen(path1, "rb");
    FILE *file2 = fopen(path2, "rb");

    if (!file1 || !file2) {
        if (file1) fclose(file1);
        if (file2) fclose(file2);
        return 0; // Can't compare, treat as non-duplicate
    }

    int result = 1;
    char buffer1[1024];
    char buffer2[1024];
    size_t size1, size2;

    do {
        size1 = fread(buffer1, 1, sizeof(buffer1), file1);
        size2 = fread(buffer2, 1, sizeof(buffer2), file2);

        if (size1 != size2 || memcmp(buffer1, buffer2, size1) != 0) {
            result = 0;
            break;
        }
    } while (size1 > 0 && size2 > 0);

    fclose(file1);
    fclose(file2);

    return result;
}

void find_duplicates(const char *dir_path) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;

    if ((dir = opendir(dir_path)) == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            find_duplicates(path);
        } else if (entry->d_type == DT_REG) {
            if (stat(path, &file_stat) == -1) {
                perror("stat");
                continue;
            }

            strncpy(files[file_count].path, path, sizeof(files[file_count].path) - 1);
            files[file_count].size = file_stat.st_size;
            file_count++;
        }
    }
    closedir(dir);
}

void compare_and_print_files() {
    int *printed = (int *)calloc(file_count, sizeof(int));
    int group_found = 0;

    for (int i = 0; i < file_count; i++) {
        if (printed[i]) continue;

        printf("---\n");
        
        group_found = 0;
        for (int j = i + 1; j < file_count; j++) {
            if (files[i].size == files[j].size && compare_files(files[i].path, files[j].path)) {
                if (!printed[i]) {
                    printf("%s\n", files[i].path);
                    printed[i] = 1;
                }
                printf("%s\n", files[j].path);
                printed[j] = 1;
                group_found = 1;
            }
        }
    }
    free(printed);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory1> <directory2> ...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        find_duplicates(argv[i]);
    }

    compare_and_print_files();
    return 0;
}
