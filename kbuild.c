#define KBUILD_H_IMPL
#include "kbuild.h"

int main(int argc, char** argv) {
    kbuild_compile_files_in_dir("src", "build");
    printf("%d\n", kbuild_is_dir("src"));
    return 0;
}
