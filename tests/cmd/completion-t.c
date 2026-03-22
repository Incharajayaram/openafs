/*
 * Validate generated shell completion files.
 */

#include <afsconfig.h>
#include <afs/param.h>

#include <roken.h>

#include <tests/tap/basic.h>

#include "common.h"

static void
check_completion_file(const char *relpath, const char *desc)
{
    char *path = afstest_src_path((char *)relpath);
    if (path == NULL) {
        ok(0, "%s: could not build path", desc);
        return;
    }

    ok(access(path, R_OK) == 0, "%s: file exists", desc);
    free(path);
}

static void
check_completion_contains(const char *relpath, const char *needle,
                          const char *desc)
{
    char *path = afstest_src_path((char *)relpath);
    if (path == NULL) {
        ok(0, "%s: could not build path", desc);
        return;
    }

    ok(afstest_file_contains(path, (char *)needle), "%s", desc);
    free(path);
}

static int
command_available(const char *command)
{
    char *cmd;
    int status;

    cmd = afstest_asprintf("command -v %s >/dev/null 2>&1", command);
    status = system(cmd);
    free(cmd);

    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void
check_shell_syntax(const char *shell, const char *relpath, const char *desc)
{
    char *path;
    char *cmd;
    struct afstest_cmdinfo cmdinfo;

    path = afstest_src_path((char *)relpath);
    if (path == NULL) {
        ok(0, "%s: could not build path", desc);
        return;
    }

    cmd = afstest_asprintf("%s -n '%s' >/dev/null 2>&1 && echo ok", shell,
                           path);

    memset(&cmdinfo, 0, sizeof(cmdinfo));
    cmdinfo.command = cmd;
    cmdinfo.exit_code = 0;
    cmdinfo.fd = STDOUT_FILENO;
    cmdinfo.output = "ok\n";

    is_command(&cmdinfo, "%s", desc);

    free(cmd);
    free(path);
}

static void
check_bash_completion(const char *relpath, const char *func,
                      const char *words, int cword, const char *needle,
                      const char *expected, const char *desc)
{
    char *path;
    char *cmd;
    struct afstest_cmdinfo cmdinfo;

    path = afstest_src_path((char *)relpath);
    if (path == NULL) {
        ok(0, "%s: could not build path", desc);
        return;
    }

    cmd = afstest_asprintf(
        "bash -lc '. \"%s\"; COMP_WORDS=(%s); COMP_CWORD=%d; %s >/dev/null 2>&1; "
        "if printf \"%%s\\n\" \"${COMPREPLY[@]}\" | grep -Fqx -- \"%s\"; then "
        "echo present; else echo absent; fi'",
        path, words, cword, func, needle);

    memset(&cmdinfo, 0, sizeof(cmdinfo));
    cmdinfo.command = cmd;
    cmdinfo.exit_code = 0;
    cmdinfo.fd = STDOUT_FILENO;
    cmdinfo.output = expected;

    is_command(&cmdinfo, "%s", desc);

    free(cmd);
    free(path);
}

int
main(int argc, char **argv)
{
    plan(23);

    check_completion_file("src/cmd/completions/bash/fs.bash", "fs bash completion");
    check_completion_file("src/cmd/completions/bash/vos.bash", "vos bash completion");
    check_completion_file("src/cmd/completions/bash/kas.bash", "kas bash completion");
    check_completion_file("src/cmd/completions/zsh/_fs", "fs zsh completion");
    check_completion_file("src/cmd/completions/zsh/_vos", "vos zsh completion");
    check_completion_file("src/cmd/completions/zsh/_kas", "kas zsh completion");

    check_shell_syntax("bash", "src/cmd/completions/bash/fs.bash",
                       "fs bash completion parses");
    check_shell_syntax("bash", "src/cmd/completions/bash/vos.bash",
                       "vos bash completion parses");
    check_shell_syntax("bash", "src/cmd/completions/bash/kas.bash",
                       "kas bash completion parses");

    check_bash_completion("src/cmd/completions/bash/vos.bash",
                          "_openafs_vos", "vos re", 1, "release",
                          "present\n",
                          "vos bash suggests subcommands");
    check_bash_completion("src/cmd/completions/bash/fs.bash",
                          "_openafs_fs", "fs sa", 1, "sa",
                          "present\n",
                          "fs bash suggests command aliases");
    check_bash_completion("src/cmd/completions/bash/vos.bash",
                          "_openafs_vos", "vos --he", 1, "--help",
                          "absent\n",
                          "vos bash omits hidden built-in commands");
    check_bash_completion("src/cmd/completions/bash/vos.bash",
                          "_openafs_vos", "vos release -f", 2, "-f",
                          "present\n",
                          "vos bash includes option aliases");
    check_bash_completion("src/cmd/completions/bash/vos.bash",
                          "_openafs_vos", "vos release -fo", 2,
                          "-force-reclone", "present\n",
                          "vos bash includes long-form options");
    check_bash_completion("src/cmd/completions/bash/vos.bash",
                          "_openafs_vos", "vos release -id foo -i", 4,
                          "-id", "absent\n",
                          "vos bash suppresses used single-value options");
    check_bash_completion("src/cmd/completions/bash/fs.bash",
                          "_openafs_fs", "fs setacl -a", 2, "-acl",
                          "present\n",
                          "fs bash suggests list-valued options");
    check_bash_completion("src/cmd/completions/bash/fs.bash",
                          "_openafs_fs",
                          "fs setacl -acl user:rlidwka -a", 4, "-acl",
                          "present\n",
                          "fs bash keeps repeatable list options available");

    check_completion_contains("src/cmd/completions/zsh/_fs",
                              "compdef _openafs_fs fs",
                              "fs zsh compdef is present");
    check_completion_contains("src/cmd/completions/zsh/_fs",
                              "setacl\\:set access control list",
                              "zsh subcommand descriptions are generated");
    check_completion_contains("src/cmd/completions/zsh/_kas",
                              "compdef _openafs_kas kas",
                              "kas zsh compdef is present");
    check_completion_contains("src/cmd/completions/zsh/_fs",
                              "-acl[access list entries]",
                              "zsh option help text is generated");

    if (command_available("zsh")) {
        check_shell_syntax("zsh", "src/cmd/completions/zsh/_fs",
                           "fs zsh completion parses");
        check_shell_syntax("zsh", "src/cmd/completions/zsh/_vos",
                           "vos zsh completion parses");
        check_shell_syntax("zsh", "src/cmd/completions/zsh/_kas",
                           "kas zsh completion parses");
    } else {
        skip("zsh not installed; skipping fs zsh syntax test");
        skip("zsh not installed; skipping vos zsh syntax test");
        skip("zsh not installed; skipping kas zsh syntax test");
    }

    return 0;
}
