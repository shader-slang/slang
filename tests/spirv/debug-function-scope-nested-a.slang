// First intermediate include level for debug-function-scope-nested.slang. No //TEST: directive.

#include "debug-function-scope-nested-leaf.slang"

float nestedUseA(float x)
{
    return nestedLeafFn(x);
}
