#pragma once
#include <stdio.h>
#include <stdlib.h>

/**
 * Reads the entire content of a text file into a null-terminated string.
 * The returned pointer must be freed by the caller.
 */
char* read_file_to_string(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    // Seek to end to get file size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    // Allocate buffer (+1 for null terminator)
    char* buffer = malloc(length + 1);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    // Read file content into buffer
    size_t read_size = fread(buffer, 1, length, file);
    buffer[read_size] = '\0';  // Null-terminate

    fclose(file);
    return buffer;
}

