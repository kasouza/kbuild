#define KBUILD_H_IMPL
#include "kbuild.h"

#include "test.h"
#define TEST_FOREACH_MAX_FILE_COUNT 8
#define TEST_FOREACH_MAX_FILE_NAME_SIZE 1024
#define TEST_FOREACH_EXPECTED_FILE_COUNT 5

KtestResult test_foreach_file() {
    const char *expected_files[TEST_FOREACH_EXPECTED_FILE_COUNT] = {
        "tests/fake-file-structure/abc",
        "tests/fake-file-structure/def",
        "tests/fake-file-structure/ghi.txt",
        "tests/fake-file-structure/very",
        "tests/fake-file-structure/verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename_verylongfilename",
    };

    char files[TEST_FOREACH_MAX_FILE_COUNT][TEST_FOREACH_MAX_FILE_NAME_SIZE];
    int files_len = 0;

    KBUILD_FOREACH_FILE("tests/fake-file-structure", {
        strncpy(files[files_len], file_info.full_path, TEST_FOREACH_MAX_FILE_NAME_SIZE);
        files_len++;
    });

    KTEST_ASSERT_EQ(TEST_FOREACH_EXPECTED_FILE_COUNT, files_len, "Should find the correct number of files");

    for (int i = 0; i < TEST_FOREACH_EXPECTED_FILE_COUNT; i++) {
        int found = 0;
        for (int j = 0; j < files_len; j++) {
            if (strcmp(expected_files[i], files[j]) == 0) {
                found = 1;
                break;
            }
        }

        KTEST_ASSERT(found, "All the expecetd files should be found");
    }

    return KTEST_RESULT_OK;
}

KtestResult test_string_builder() {
    KbuildStringBuilder *builder = kbuild_create_string_builder();

    const char* expected_str = "saske\tnarutosakurãdasjkldjaskd\n\nllllkliveryloongworkkkkkkkkdddasdsax";
    kbuild_string_builder_append(builder, "saske\t");
    kbuild_string_builder_append(builder, "naruto");
    kbuild_string_builder_append(builder, "sakurã");
    kbuild_string_builder_append(builder, "dasjkldjaskd\n\nllll");

    kbuild_string_builder_append_ch(builder, 'k');
    kbuild_string_builder_append_ch(builder, 'l');
    kbuild_string_builder_append_ch(builder, 'i');

    kbuild_string_builder_append(builder, "veryloongworkkkkkkkkdddasdsa");

    kbuild_string_builder_append_ch(builder, 'x');

    char *str = kbuild_string_builder_build(builder);
    KTEST_ASSERT_EQ_STR(str, expected_str, "Must build the correct string");

    free(str);
    kbuild_free_string_builder(builder);

    // Just to check if the capacity is growing correclty with chars
    KbuildStringBuilder *builder2 = kbuild_create_string_builder();
    const char expected_str2[65] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    for (int i = 0; i < 64; i++) {
        kbuild_string_builder_append_ch(builder, 'x');
    }

    char *str2 = kbuild_string_builder_build(builder2);

    KTEST_ASSERT_EQ_STR(str2, expected_str2, "Must build the correct string");
    KTEST_ASSERT_EQ(builder2->cap, 64, "Should have the capacity for 64 characters in the buffer");

    free(str2);
    kbuild_free_string_builder(builder2);

    // Test reverse build
    KbuildStringBuilder *builder_reverse = kbuild_create_string_builder();
    const char *expected_string_reverse = "dcba";

    kbuild_string_builder_append_ch(builder_reverse, 'a');
    kbuild_string_builder_append_ch(builder_reverse, 'b');
    kbuild_string_builder_append_ch(builder_reverse, 'c');
    kbuild_string_builder_append_ch(builder_reverse, 'd');

    char *str_reverse = kbuild_string_builder_build_reverse(builder_reverse);

    KTEST_ASSERT_EQ_STR(expected_string_reverse, str_reverse, "Should a return a string in the reverse order that it was built");

    free(str_reverse);
    kbuild_free_string_builder(builder_reverse);

    // Test empty builder
    KbuildStringBuilder *builder_empty = kbuild_create_string_builder();
    char *str_empty1 = kbuild_string_builder_build(builder_empty);
    char *str_empty2 = kbuild_string_builder_build_reverse(builder_empty);
    KTEST_ASSERT_EQ_STR(str_empty1, "", "Should return an empty string for the normal build");
    KTEST_ASSERT_EQ_STR(str_empty2, "", "Should return an empty string for the reverse build");

    free(str_empty1);
    free(str_empty2);
    kbuild_free_string_builder(builder_empty);

    // Test clearing the builder
    KbuildStringBuilder *builder_clear = kbuild_create_string_builder();
    kbuild_string_builder_append(builder_clear, "somerandomstuff");

    char *str_not_clear = kbuild_string_builder_build(builder_clear);
    KTEST_ASSERT_EQ_STR(str_not_clear, "somerandomstuff", "Should return the correct string");

    kbuild_string_builder_clear(builder_clear);

    char *str_clear = kbuild_string_builder_build(builder_clear);
    KTEST_ASSERT_EQ_STR(str_clear, "", "Should return an empty string for the normal build");

    free(str_not_clear);
    free(str_clear);
    kbuild_free_string_builder(builder_clear);

    // Test copying n bytes
    KbuildStringBuilder *builder_n_bytes = kbuild_create_string_builder();
    kbuild_string_builder_appendn(builder_n_bytes, "somerandomstuff", 4);

    char *str_n_bytes = kbuild_string_builder_build(builder_n_bytes);
    KTEST_ASSERT_EQ_STR(str_n_bytes, "some", "Should add only the 4 first bytes to the final string");

    free(str_n_bytes);
    kbuild_free_string_builder(builder_n_bytes);

    // Test copying n bytes (way more than the size of the string)
    KbuildStringBuilder *builder_n_bytes2 = kbuild_create_string_builder();
    kbuild_string_builder_appendn(builder_n_bytes2, "somerandomstuf", 999999);

    char *str_n_bytes2 = kbuild_string_builder_build(builder_n_bytes2);
    KTEST_ASSERT_EQ_STR(str_n_bytes2, "somerandomstuf", "Should return the complete string");

    free(str_n_bytes);
    kbuild_free_string_builder(builder_n_bytes2);

    return KTEST_RESULT_OK;
}

KtestResult test_pathinfo() {
    KbuildPathInfo *pathinfo = kbuild_pathinfo("/saske/narutos/sakuras/file.old.zip");

    KTEST_ASSERT_EQ_STR(pathinfo->extension, "zip", "Should return the correct extension");
    KTEST_ASSERT_EQ_STR(pathinfo->basename, "file.old.zip", "Should return the correct basename");
    KTEST_ASSERT_EQ_STR(pathinfo->filename, "file.old", "Should return the correct filename");
    KTEST_ASSERT_EQ_STR(pathinfo->dirname, "/saske/narutos/sakuras", "Should return the correct dir name");

    kbuild_free_pathinfo(pathinfo);
    pathinfo = NULL;

    // Test witho no directory
    KbuildPathInfo *pathinfo_nodir = kbuild_pathinfo("file.old.zip");
    KTEST_ASSERT_EQ_STR(pathinfo_nodir->extension, "zip", "Should return the correct extension");
    KTEST_ASSERT_EQ_STR(pathinfo_nodir->basename, "file.old.zip", "Should return the correct basename");
    KTEST_ASSERT_EQ_STR(pathinfo_nodir->filename, "file.old", "Should return the correct filename");
    KTEST_ASSERT_EQ_STR(pathinfo_nodir->dirname, "", "Should return the correct dir name");

    kbuild_free_pathinfo(pathinfo_nodir);
    pathinfo_nodir = NULL;

    // Test witho no extension
    KbuildPathInfo *pathinfo_noext = kbuild_pathinfo("file");
    KTEST_ASSERT_EQ_STR(pathinfo_noext->extension, "", "Should return the correct extension");
    KTEST_ASSERT_EQ_STR(pathinfo_noext->basename, "file", "Should return the correct basename");
    KTEST_ASSERT_EQ_STR(pathinfo_noext->filename, "file", "Should return the correct filename");
    KTEST_ASSERT_EQ_STR(pathinfo_noext->dirname, "", "Should return the correct dir name");

    KbuildPathInfo *pathinfo_empty = kbuild_pathinfo("");
    KTEST_ASSERT_EQ(pathinfo_empty, NULL, "Should return a NULL pointer if the path is empty");
}

KtestResult test_join_paths() {
    const char *paths[] = {"/saske/", "naruto/", "", "/fernando", "maria/"};
    char *path = kbuild_join_paths(paths, 5);
    KTEST_ASSERT_EQ_STR(path, "/saske/naruto/fernando/maria", "Should properly join the paths");
    free(path);

    // Test 2
    const char *paths2[] = {"/"};
    char *path2 = kbuild_join_paths(paths2, 1);
    KTEST_ASSERT_EQ_STR(path2, "/", "Should properly join the paths");
    free(path2);

    // Test 3
    const char *paths3[] = {""};
    char *path3 = kbuild_join_paths(paths3, 1);
    KTEST_ASSERT_EQ_STR(path3, "", "Should properly join the paths");
    free(path3);

    // Test 4
    const char *paths4[] = {"/", "dir"};
    char *path4 = kbuild_join_paths(paths4, 2);
    KTEST_ASSERT_EQ_STR(path4, "/dir", "Should properly join the paths");
    free(path4);

    //  Test 5
    const char *paths5[] = {"/", "dir", "/", "dasl"};
    char *path5 = kbuild_join_paths(paths5, 4);
    KTEST_ASSERT_EQ_STR(path5, "/dir/dasl", "Should properly join the paths");
    free(path5);

    // Test 6
    const char *paths6[] = {"/", "/"};
    char *path6 = kbuild_join_paths(paths6, 2);
    KTEST_ASSERT_EQ_STR(path6, "/", "Should properly join the paths");
    free(path6);

    // Test 7
    const char *paths7[] = {"../", "file"};
    char *path7 = kbuild_join_paths(paths7, 2);
    KTEST_ASSERT_EQ_STR(path7, "../file", "Should properly join the paths");
    free(path7);

    // Test 8
    const char *paths8[] = {"../", "/file/somemore/"};
    char *path8 = kbuild_join_paths(paths8, 2);
    KTEST_ASSERT_EQ_STR(path8, "../file/somemore", "Should properly join the paths");
    free(path8);

    // Test 9
    const char *paths9[] = {"/file/"};
    char *path9 = kbuild_join_paths(paths9, 1);
    KTEST_ASSERT_EQ_STR(path9, "/file", "Should properly join the paths");
    free(path9);

    // Test 10
    const char *paths10[] = {"file/"};
    char *path10 = kbuild_join_paths(paths10, 1);
    KTEST_ASSERT_EQ_STR(path10, "file", "Should properly join the paths");
    free(path10);
}

KtestResult test_join_strs() {
    const char *strs[] = { "abc", "def", "ghi\n", "what\tever" };
    char *joined = kbuild_join_strs(strs, 4);
    KTEST_ASSERT_EQ_STR(joined, "abcdefghi\nwhat\tever", "Should properly join the strings");
    free(joined);

    const char *strs2[] = { "", "def", "ghi\n", "what\tever" };
    char *joined2 = kbuild_join_strs(strs2, 4);
    KTEST_ASSERT_EQ_STR(joined2, "defghi\nwhat\tever", "Should properly join the strings");
    free(joined2);

    const char *strs3[] = { "", "", "" };
    char *joined3 = kbuild_join_strs(strs3, 3);
    KTEST_ASSERT_EQ_STR(joined3, "", "Should properly join the strings");
    free(joined3);

    const char *strs4[] = { "" };
    char *joined4 = kbuild_join_strs(strs4, 1);
    KTEST_ASSERT_EQ_STR(joined4, "", "Should properly join the strings");
    free(joined4);
}

KtestResult test_dyn_array() {
    KBUILD_DYNARR(int) *arr = KBUILD_CREATE_DYNARR(int);

    for (int i = 0; i < 128; i++) {
        KBUILD_DYNARR_PUSH_BACK(arr, i * i);
    }

    for (int i = 0; i < 128; i++) {
        KTEST_ASSERT_EQ(KBUILD_DYNARR_AT(arr, i), i * i, "Should get the correct value from the array");
    }

    for (int i = 0; i < 128; i++) {
        KBUILD_DYNARR_SET(arr, i, i - 3);
    }

    for (int i = 0; i < 128; i++) {
        KTEST_ASSERT_EQ(KBUILD_DYNARR_AT(arr, i), i - 3, "Should get the correct value from the array");
    }

    KBUILD_FREE_DYNARR(arr);

    // Test dynnar append
    KBUILD_DYNARR(int) * arr2 = KBUILD_CREATE_DYNARR(int);
    KBUILD_DYNARR(int) * arr3 = KBUILD_CREATE_DYNARR(int);
    
    KBUILD_DYNARR_PUSH_BACK(arr2, 1);
    KBUILD_DYNARR_PUSH_BACK(arr2, 2);
    KBUILD_DYNARR_PUSH_BACK(arr2, 3);

    KBUILD_DYNARR_PUSH_BACK(arr3, 4);
    KBUILD_DYNARR_PUSH_BACK(arr3, 5);
    KBUILD_DYNARR_PUSH_BACK(arr3, 6);

    KBUILD_DYNARR_APPEND(arr2, arr3);

    KTEST_ASSERT_EQ(KBUILD_DYNARR_LEN(arr2), 6, "Should have properly merged the two arrays");
    KTEST_ASSERT_EQ(KBUILD_DYNARR_AT(arr2, 0), 1, "Should have properly merged the two arrays");
    KTEST_ASSERT_EQ(KBUILD_DYNARR_AT(arr2, 1), 2, "Should have properly merged the two arrays");
    KTEST_ASSERT_EQ(KBUILD_DYNARR_AT(arr2, 2), 3, "Should have properly merged the two arrays");
    KTEST_ASSERT_EQ(KBUILD_DYNARR_AT(arr2, 3), 4, "Should have properly merged the two arrays");
    KTEST_ASSERT_EQ(KBUILD_DYNARR_AT(arr2, 4), 5, "Should have properly merged the two arrays");
    KTEST_ASSERT_EQ(KBUILD_DYNARR_AT(arr2, 5), 6, "Should have properly merged the two arrays");

    KBUILD_DYNARR(int) * arr_empty = KBUILD_CREATE_DYNARR(int);
    KBUILD_DYNARR_APPEND(arr2, arr_empty);
    KTEST_ASSERT_EQ(KBUILD_DYNARR_LEN(arr2), 6, "Appending an empty array should not change the original");

    KBUILD_FREE_DYNARR(arr2);
    KBUILD_FREE_DYNARR(arr3);
}

int main() {
    KTEST(test_foreach_file);
    KTEST(test_string_builder);
    KTEST(test_pathinfo);
    KTEST(test_join_paths);
    KTEST(test_join_strs);
    KTEST(test_dyn_array);

    return 0;
}
