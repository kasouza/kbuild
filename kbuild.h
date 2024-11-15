#ifndef KBUILD_H
#define KBUILD_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <assert.h>

#define KBUILD_OBJECT_FILE_EXTENSION ".o"
#define KBUILD_DIRECTORY_SEPARATOR '/'
#define KBUILD_EXTENSION_SEPARATOR '.'
#define KBUILD_FILE_INFO_FULL_PATH_SIZE 1024
#define KBUILD_MAX_COMMAND_SIZE 1024
#define KBUILD_MAX_OUTPUT_FULLPATH_SIZE 1024
#define KBUILD_MAX_EXTENSION_SIZE 32
#define KBUILD_CC "cc"
#define KBUILD_CFLAGS ""
#define KBUILD_STRING_BUILDER_INITIAL_SIZE 32
#define KBUILD_STRING_BUILDER_SCALE_FACTOR 2

#define KBUILD_ERRORF(code, format, ...) \
    do { \
        fprintf(stderr, "ERROR[%s, %d] "  #code  " " format, __FILE__, __LINE__, __VA_ARGS__); \
        exit(code); \
    } while (0)

#define KBUILD_ERROR(code) \
    do { \
        fprintf(stderr, "ERROR[%s, %d] "  #code "\n", __FILE__, __LINE__); \
        exit(code); \
    } while (0)

#define KBUILD_FOREACH_FILE(dir_path, body) \
    do { \
        DIR *d; \
        struct dirent *dir; \
        d = opendir(dir_path); \
        if (!d) { \
            KBUILD_ERRORF(KBUILD_ERROR_FILE_NOT_FOUND, "%s\n", dir_path); \
        } \
        KbuildFileInfo file_info; \
        size_t dir_path_len = strlen(dir_path); \
        if (dir_path_len >= KBUILD_FILE_INFO_FULL_PATH_SIZE) { \
            KBUILD_ERRORF(KBUILD_ERROR_PATH_TOO_BIG, "%s\n", dir_path); \
        } \
        strncpy(file_info.full_path, dir_path, KBUILD_FILE_INFO_FULL_PATH_SIZE); \
        while ((dir = readdir(d)) != NULL) { \
            if (dir->d_name[0] == '.') { \
                continue; \
            } \
            { \
                file_info.name = dir->d_name; \
                long file_name_len = strlen(file_info.name); \
                \
                if ((file_name_len + dir_path_len + 1) >= KBUILD_FILE_INFO_FULL_PATH_SIZE) { \
                    KBUILD_ERRORF(KBUILD_ERROR_PATH_TOO_BIG, "%s/%s\n", dir_path, file_info.name); \
                } \
                \
                memcpy(file_info.full_path + dir_path_len + 1,  file_info.name, file_name_len); \
                file_info.full_path[dir_path_len] = KBUILD_DIRECTORY_SEPARATOR; \
                file_info.full_path[dir_path_len + file_name_len + 1] = '\0'; \
                \
                file_info.is_dir = kbuild_is_dir(file_info.full_path); \
            } \
            \
            body \
        } \
        closedir(d); \
    } while(0)

typedef struct {
    const char *name;
    char full_path[KBUILD_FILE_INFO_FULL_PATH_SIZE];
    int is_dir;
} KbuildFileInfo;

typedef enum {
    KBUILD_ERROR_FILE_NOT_FOUND = 1,
    KBUILD_ERROR_PATH_TOO_BIG = 2,
    KBUILD_ERROR_COMPILING = 3,
    KBUILD_ERROR_OUTPUT_FILE_PATH_TOO_BIG = 4,
    KBUILD_ERROR_EXTENSION_SIZE_TOO_BIG = 5,
    KBUILD_ERROR_INVALID_EXTENSION = 6,
    KBUILDER_ERROR_INVALID_PATH = 7
} KbuildError;

typedef struct {
    char *dirname;
    char *extension;
    char *basename;
    char *filename;
} KbuildPathInfo;

typedef struct {
    char *buffer;
    int len;
    int cap;
} KbuildStringBuilder;

void kbuild_compile(const char* input_path, const char*output_path);
int kbuild_is_dir(const char* path);
void kbuild_compile_files_in_dir(const char* path, const char* build_path);

KbuildPathInfo *kbuild_pathinfo(const char* path);
void kbuild_free_pathinfo(KbuildPathInfo  *pathinfo);

KbuildStringBuilder* kbuild_create_string_builder();
void kbuild_string_builder_append(KbuildStringBuilder * builder, const char* str);
void kbuild_string_builder_appendn(KbuildStringBuilder * builder, const char* str, int n);
void kbuild_string_builder_append_ch(KbuildStringBuilder * builder, char ch);

char *kbuild_join_paths(const char** paths, int paths_len);

/**
  * Builds a char* from the internal buffer
  * The returned pointer should be freed by the caller
  */
char * kbuild_string_builder_build(KbuildStringBuilder *builder);

/**
  * Builds a char* from the internal buffer in reverse order
  * The returned pointer should be freed by the caller
  */
char * kbuild_string_builder_build_reverse(KbuildStringBuilder *builder);

void kbuild_string_builder_clear(KbuildStringBuilder *builder);

void kbuild_free_string_builder(KbuildStringBuilder *string_buider);

#endif

#ifdef KBUILD_H_IMPL

int kbuild_is_dir(const char* path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return 0;
    }

    return S_ISDIR(path_stat.st_mode);
}

KbuildPathInfo* kbuild_pathinfo(const char* path) {
    assert(path != NULL);

    int len = strlen(path);
    if (len == 0) {
        return NULL;
    }

    KbuildStringBuilder *dirname_builder = kbuild_create_string_builder();
    KbuildStringBuilder *basename_builder = kbuild_create_string_builder();
    KbuildStringBuilder *filename_builder = kbuild_create_string_builder();
    KbuildStringBuilder *extension_builder = kbuild_create_string_builder();

    int found_extension = 0;
    int found_basename = 0;
    int found_filename = 0;

    for (int i = len - 1; i >= 0; i--) {
        char ch = path[i];

        if (!found_extension) {
            if (ch == KBUILD_EXTENSION_SEPARATOR) {
                found_extension = 1;
            } else {
                kbuild_string_builder_append_ch(extension_builder, ch);
            }
        } else if (!found_filename) {
            if (ch == KBUILD_DIRECTORY_SEPARATOR) {
                found_filename = 1;
            } else {
                kbuild_string_builder_append_ch(filename_builder, ch);
            }
        }

        if (!found_basename) {
            if (ch == KBUILD_DIRECTORY_SEPARATOR) {
                found_basename = 1;
            } else {
                kbuild_string_builder_append_ch(basename_builder, ch);
            }
        } else {
            kbuild_string_builder_append_ch(dirname_builder, ch);
        }
    }


    KbuildPathInfo *pathinfo = malloc(sizeof(KbuildPathInfo));

    pathinfo->dirname = kbuild_string_builder_build_reverse(dirname_builder);
    pathinfo->basename = kbuild_string_builder_build_reverse(basename_builder);

    if (!found_extension) {
        kbuild_string_builder_clear(extension_builder);

        kbuild_string_builder_append(filename_builder, pathinfo->basename);
        pathinfo->filename = kbuild_string_builder_build(filename_builder);
    } else {
        pathinfo->filename = kbuild_string_builder_build_reverse(filename_builder);
    }

    pathinfo->extension = kbuild_string_builder_build_reverse(extension_builder);

    kbuild_free_string_builder(dirname_builder);
    kbuild_free_string_builder(basename_builder);
    kbuild_free_string_builder(filename_builder);
    kbuild_free_string_builder(extension_builder);

    return pathinfo;
}

void kbuild_free_pathinfo(KbuildPathInfo* pathinfo) {
    assert(pathinfo != NULL);
    assert(pathinfo->dirname != NULL);
    assert(pathinfo->basename != NULL);
    assert(pathinfo->extension != NULL);
    assert(pathinfo->dirname != NULL);

    free(pathinfo->dirname);
    free(pathinfo->basename);
    free(pathinfo->extension);
    free(pathinfo->filename);
    free(pathinfo);
}

KbuildStringBuilder* kbuild_create_string_builder() {
    KbuildStringBuilder *builder = malloc(sizeof(KbuildStringBuilder));
    builder->buffer = malloc(KBUILD_STRING_BUILDER_INITIAL_SIZE);
    builder->cap = KBUILD_STRING_BUILDER_INITIAL_SIZE;
    builder->len = 0;

    return builder;
}

void kbuild_string_builder_append(KbuildStringBuilder * builder, const char* str) {
    kbuild_string_builder_appendn(builder, str, strlen(str));
}

void kbuild_string_builder_appendn(KbuildStringBuilder * builder, const char* str, int n) {
    assert(builder != NULL);
    assert(builder->buffer != NULL);
    assert(str != NULL);

    int other_len = strlen(str);
    assert(other_len > 0);
    assert(n > 0);

    int bytes_to_copy = n > other_len ? other_len : n;

    int new_len = builder->len + bytes_to_copy;

    if (new_len > builder->cap) {
        int new_cap = builder->cap * KBUILD_STRING_BUILDER_SCALE_FACTOR;

        builder->buffer = realloc(builder->buffer, new_cap);
        builder->cap = new_cap;
    }

    memcpy(builder->buffer + builder->len, str, bytes_to_copy);
    builder->len = new_len;
}

void kbuild_string_builder_append_ch(KbuildStringBuilder * builder, char ch) {
    assert(builder != NULL);
    assert(builder->buffer != NULL);
    assert(ch != '\0');

    int other_len = 1;
    int new_len = builder->len + other_len;

    if (new_len > builder->cap) {
        int new_cap = builder->cap * KBUILD_STRING_BUILDER_SCALE_FACTOR;

        builder->buffer = realloc(builder->buffer, new_cap);
        builder->cap = new_cap;
    }

    builder->buffer[builder->len] = ch;
    builder->len = new_len;
}

char * kbuild_string_builder_build(KbuildStringBuilder * builder) {
    assert(builder != NULL);
    assert(builder->buffer != NULL);

    char *str = malloc(builder->len + 1);
    if (builder->len > 0) {
        memcpy(str, builder->buffer, builder->len);
    }
    str[builder->len] = '\0';

    return str;
}

char * kbuild_string_builder_build_reverse(KbuildStringBuilder * builder) {
    assert(builder != NULL);
    assert(builder->buffer != NULL);

    char *str = malloc(builder->len + 1);
    if (builder->len > 0) {
        int remaining_buffer = builder->len + (builder->cap - builder->len);
        for (int i = 0; i < builder->len; i++) {
            str[i] = builder->buffer[builder->len - i - 1];
        }
    }

    str[builder->len] = '\0';

    return str;
}

void kbuild_string_builder_clear(KbuildStringBuilder *builder) {
    assert(builder != NULL);
    assert(builder->buffer != NULL);
    builder->len = 0;
}

void kbuild_free_string_builder(KbuildStringBuilder *string_builder) {
    assert(string_builder != NULL);
    assert(string_builder->buffer != NULL);

    free(string_builder->buffer);
    free(string_builder);
}

char *kbuild_join_paths(const char** paths, int paths_len) {
    KbuildStringBuilder *builder = kbuild_create_string_builder();
    if (builder == NULL) {
        return NULL;
    }

    int are_all_paths_just_separators = 1;

    for (int i = 0; i < paths_len; i++) {
        const char *path = paths[i];
        int len = strlen(path);

        if (!(len == 1 && path[0] == KBUILD_DIRECTORY_SEPARATOR)) {
            are_all_paths_just_separators = 0;
        }

        if (len <= 0) {
            continue;
        }

        int bytes_to_copy = len;
        if (path[bytes_to_copy - 1] == KBUILD_DIRECTORY_SEPARATOR) {
            bytes_to_copy--;
        }

        if (i > 0 && path[0] != KBUILD_DIRECTORY_SEPARATOR) {
            kbuild_string_builder_append_ch(builder, KBUILD_DIRECTORY_SEPARATOR);
        }

        if (bytes_to_copy > 0) {
            kbuild_string_builder_appendn(builder, path, bytes_to_copy);
        }
    }

    if (are_all_paths_just_separators) {
        kbuild_string_builder_clear(builder);
        kbuild_string_builder_append_ch(builder, KBUILD_DIRECTORY_SEPARATOR);
    }

    char *full_path = kbuild_string_builder_build(builder);

    kbuild_free_string_builder(builder);

    return full_path;
}

void kbuild_compile_files_in_dir(const char* input_path, const char* build_path) {
    int build_path_len = strlen(build_path);

    // length of the build path + separator
    if ((build_path_len + 1) >= KBUILD_MAX_OUTPUT_FULLPATH_SIZE) {
        KBUILD_ERRORF(KBUILD_ERROR_OUTPUT_FILE_PATH_TOO_BIG, "For build path %s\n", build_path);
    }

    int output_extension_len = strlen(KBUILD_OBJECT_FILE_EXTENSION);

    KBUILD_FOREACH_FILE(input_path, {
        if (file_info.is_dir) {
            kbuild_compile_files_in_dir(file_info.full_path, build_path);
        } else {
            KbuildPathInfo *pathinfo = kbuild_pathinfo(file_info.full_path);
            if (pathinfo == NULL) {
                KBUILD_ERRORF(KBUILDER_ERROR_INVALID_PATH, "%s\n", file_info.full_path);
            }

            const char *output_dir_paths_to_join[2];
            output_dir_paths_to_join[0] = build_path;
            output_dir_paths_to_join[1] = pathinfo->dirname;

            char *output_full_dir_path = kbuild_join_paths(output_dir_paths_to_join, 2);
            
            // TODO: Implement a proper mkdir
            char cmd[1024] = "";
            sprintf(cmd, "mkdir %s -p", output_full_dir_path);
            system(cmd);

            // TODO: A general joins for non path string
            KbuildStringBuilder *basename_builder = kbuild_create_string_builder();
            kbuild_string_builder_append(basename_builder, pathinfo->filename);
            kbuild_string_builder_append(basename_builder, KBUILD_OBJECT_FILE_EXTENSION);

            char *output_basename = kbuild_string_builder_build(basename_builder);
            
            free(basename_builder);
            basename_builder = NULL;

            // TODO: Make this a little nicer to use
            const char *output_file_paths_to_join[2];
            output_file_paths_to_join[0] = output_full_dir_path;
            output_file_paths_to_join[1] = output_basename;

            char *output_full_file_path = kbuild_join_paths(output_file_paths_to_join, 2);

            free(output_full_dir_path);
            output_full_dir_path = NULL;

            free(output_basename);
            output_basename = NULL;

            kbuild_compile(file_info.full_path, output_full_file_path);

            kbuild_free_pathinfo(pathinfo);
        }
    });
}

void kbuild_compile(const char* input_path, const char*output_path) {
    char cmd[KBUILD_MAX_COMMAND_SIZE];
    snprintf(cmd, KBUILD_MAX_COMMAND_SIZE, "%s -c -o %s %s %s", KBUILD_CC, output_path, input_path, KBUILD_CFLAGS);
       
    if (system(cmd) != 0) {
        KBUILD_ERRORF(KBUILD_ERROR_COMPILING, "Could not compile %s\n", input_path);
    }
}

#endif