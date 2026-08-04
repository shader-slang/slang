// The outer file both intermediates are included into, and the compilation unit that owns everything
// they contribute. No //TEST: directive — passed on the command line of
// debug-function-scope-nested.slang.

#include "debug-function-scope-nested-a.slang"
#include "debug-function-scope-nested-b.slang"

float nestedOwnerFn(float x)
{
    return nestedUseA(x) + nestedUseB(x);
}
