// This is a non-test helper module for debug-source-import-include.slang (no //TEST: directive).
// It is pulled in via `#include` from debug-source-import-include-module.slang, so it exercises
// the `isIncludedFile == true` DebugSource operand path.

public float getIncludedValue()
{
    return 1.0;
}
