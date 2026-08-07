// Second top-level (non-included) source file for debug-function-scope-multifile-include.slang, and
// the file that does the #include. It is compiled together with that file, so it gets its own
// DebugCompilationUnit — distinct from the entry point's.
//
// Do not delete this file as unused, and do not move the #include into the file holding the entry
// point. The module-global scope a DebugFunction falls back to is pinned to the entry point's
// compilation unit, so if the includer and the entry point were the same file, the broken fallback
// and the correct answer would be the same inst and the test would pass while the feature is broken.
// Keeping the #include here — away from the entry point — is what makes the two differ.

#include "debug-function-scope-multifile-inc.slang"

float multifileOtherFunc(float x)
{
    return multifileHelperFromInclude(x) + 1.0;
}
