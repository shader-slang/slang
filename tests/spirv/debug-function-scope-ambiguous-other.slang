// Second includer for debug-function-scope-ambiguous.slang, and the file holding the entry point.
// No //TEST: directive — it is passed on that test's command line.
//
// The entry point lives here, not in the test file, and that placement is load-bearing. The test file
// is the first includer of the shared header, so it is the compilation unit a resolution that simply
// took the first qualifying occurrence would choose. The module-global fallback instead resolves to
// the entry point's compilation unit — this file's. Keeping those two files distinct is what makes a
// wrong binding observable; with the entry point in the test file, the wrong answer and the fallback
// would be the same instruction.

#define AMBIGUOUS_VARIANT_A
#include "debug-function-scope-ambiguous-inc.slang"

float ambiguousWrapB(float x);

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(ambiguousFromA(0.25) + ambiguousWrapB(0.5));
}
