// First of two modules that #include the same header. No //TEST: directive — imported by
// debug-function-scope-shared-header.slang.

module debug_function_scope_shared_header_a;

#include "debug-function-scope-shared-header-inc.slang"

public float wrapInA(float x)
{
    return sharedFromHeader(x);
}
