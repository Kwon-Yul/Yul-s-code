#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define TABLE_SIZE 100

typedef struct Node {
    char *filename;
    char **paths;
    int pathCount;
    struct Node *next;
} Node;

typedef struct HashTable {
    Node *buckets[TABLE_SIZE];
} HashTable;

unsigned int hash(const char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + *str++;
    }
    return hash % TABLE_SIZE;
}

HashTable *createHashTable() {
    HashTable *table = malloc(sizeof(HashTable));
    for (int i = 0; i < TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}

void insert(HashTable *table, const char *filename, const char *path) {
    unsigned int index = hash(filename);
    Node *current = table->buckets[index];
    while (current != NULL) {
        if (strcmp(current->filename, filename) == 0) {
            current->paths = realloc(current->paths, (current->pathCount + 1) * sizeof(char *));
            char *fullPath = malloc(strlen(path) + strlen(filename) + 2); // 1 for '/' and 1 for '\0'
            sprintf(fullPath, "%s/%s", path, filename);
            current->paths[current->pathCount] = fullPath;
            current->pathCount++;
            return;
        }
        current = current->next;
    }

    Node *newNode = malloc(sizeof(Node));
    newNode->filename = strdup(filename);
    newNode->paths = malloc(sizeof(char *));
    char *fullPath = malloc(strlen(path) + strlen(filename) + 2); // 1 for '/' and 1 for '\0'
    sprintf(fullPath, "%s/%s", path, filename);
    newNode->paths[0] = fullPath;
    newNode->pathCount = 1;
    newNode->next = table->buckets[index];
    table->buckets[index] = newNode;
}

void searchAndPrintDuplicates(const char *dirPath, HashTable *table) {
    DIR *dir = opendir(dirPath);
    if (!dir) {
        fprintf(stderr, "Cannot open directory: %s\n", dirPath);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char newPath[1024];
                snprintf(newPath, sizeof(newPath), "%s/%s", dirPath, entry->d_name);
                searchAndPrintDuplicates(newPath, table);
            }
        } else if (entry->d_type == DT_REG) {
            unsigned int index = hash(entry->d_name);
            insert(table, entry->d_name, dirPath);
        }
    }

    closedir(dir);
}

void printDuplicates(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = table->buckets[i];
        while (current != NULL) {
            if (current->pathCount > 1) {
                printf("---\n");
                for (int j = 0; j < current->pathCount; j++) {
                    printf("%s\n", current->paths[j]);
                }
            }
            current = current->next;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s directory1 directory2 ...\n", argv[0]);
        return 1;
    }

    HashTable *table = createHashTable();

    for (int i = 1; i < argc; i++) {
        searchAndPrintDuplicates(argv[i], table);
    }

    printDuplicates(table);

    return 0;
}

