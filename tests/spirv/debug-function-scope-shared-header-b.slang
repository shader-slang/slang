// Second of two modules that #include the same header. Keep both: with only one includer the
// per-occurrence resolution is indistinguishable from resolving by source file, and the test loses
// its power. No //TEST: directive — imported by debug-function-scope-shared-header.slang.

module debug_function_scope_shared_header_b;

#include "debug-function-scope-shared-header-inc.slang"

public float wrapInB(float x)
{
    return sharedFromHeader(x) + 1.0;
}
