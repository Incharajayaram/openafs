/*
 * Basic tests for generated bash completion files.
 */

#include <afsconfig.h>
#include <afs/param.h>

#include <roken.h>

#include <tests/tap/basic.h>
#include "common.h"

static void
check_completion_file(const char *relpath, const char *needle, const char *desc)
{
    char *path = afstest_src_path((char *)relpath);
    if (path == NULL) {
        ok(0, "%s: could not build path", desc);
        return;
    }

    ok(access(path, R_OK) == 0, "%s: file exists", desc);
    ok(afstest_file_contains(path, (char *)needle), "%s: contains expected line", desc);
    free(path);
}

int
main(int argc, char **argv)
{
    plan(8);

    check_completion_file("src/cmd/completions/bash/fs.bash",
                          "complete -F _openafs_fs fs",
                          "fs completion");

    check_completion_file("src/cmd/completions/bash/vos.bash",
                          "complete -F _openafs_vos vos",
                          "vos completion");

    check_completion_file("src/cmd/completions/zsh/_fs",
                          "compdef _openafs_fs fs",
                          "fs zsh completion");

    check_completion_file("src/cmd/completions/zsh/_vos",
                          "compdef _openafs_vos vos",
                          "vos zsh completion");

    check_completion_file("src/cmd/completions/bash/kas.bash",
                          "complete -F _openafs_kas kas",
                          "kas completion");

    check_completion_file("src/cmd/completions/zsh/_kas",
                          "compdef _openafs_kas kas",
                          "kas zsh completion");

    return 0;
}
