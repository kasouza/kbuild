#define KBUILD_H_IMPL
#include "kbuild.h"

int main(int argc, char** argv) {
    KBUILD_DYNARR(kbuild_str_t) *object_files = kbuild_compile_files_in_dir("test-src", "build");
    kbuild_link_files(object_files, "build/saske");

    for (int i = 0; i < object_files->len; i++) {
        free(object_files->buffer[i]);
    }

    KBUILD_FREE_DYNARR(object_files);

    return 0;
}
