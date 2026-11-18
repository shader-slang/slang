//TEST:EXECUTABLE:

// Cross-module regression for shader-slang/slang#14578, in the shape closest
// to the original bug: the unreferenced static with a side-effecting
// initializer lives in a SECOND module (imported here as source), so the
// per-module allowlist and its linking-info cache must preserve it while
// linking that module. `s` calls `bump()` twice at load time, and the
// compiled program must print the state those calls wrote (pinned by this
// test's .expected file); when the link dropped such globals the program
// printed 0. The harness builds its own command line for executable tests
// (fixed `-target exe` and include paths; directive arguments are not passed
// through), so no options appear here.

import "static-init-cross-module-lib";

export __extern_cpp int main()
{
    printf("cross-module-static-init=%d\n", get_state());
    return 0;
}
