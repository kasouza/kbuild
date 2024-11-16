#define KBUILD_H_IMPL
#include "kbuild.h"

int main(int argc, char** argv) {
    KBUILD_DYNARR(kbuild_str_t) *object_files = kbuild_compile_files_in_dir("test-src", "build");
    for (int i = 0; i < KBUILD_DYNARR_LEN(object_files); i++) {
        printf("%s\n", KBUILD_DYNARR_AT(object_files, i));
    }

    return 0;
}
