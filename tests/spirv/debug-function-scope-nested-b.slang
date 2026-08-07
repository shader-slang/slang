// Second intermediate include level for debug-function-scope-nested.slang. Keep both intermediates:
// with one, the leaf has a single immediate includer and the test no longer exercises chains that
// diverge before reconverging. No //TEST: directive.

#include "debug-function-scope-nested-leaf.slang"

float nestedUseB(float x)
{
    return nestedLeafFn(x) + 1.0;
}
