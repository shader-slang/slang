//TEST_IGNORE_FILE:

// Companion module for extern-overload-*.slang: defines the `int` overload of `f` (not extern).
// It imports overloadimpl without re-exporting it, so that the `float` definition is linked in but
// is not visible to the test files: their only `float` candidate is the `extern` declaration.
module overloaddef;

import overloadimpl;

public int f(int x)
{
    return x + 1;
}
