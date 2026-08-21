// Regression test for shader-slang/slang#12668, cross-module case: an attribute declared inside a
// namespace in an imported module must resolve through its qualified path even when the importing
// module re-opens the same namespace (producing sibling scopes of a merged namespace).

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -no-codegen

module test;

import attrmod;

// Re-open the same namespace here so the importing module contributes a sibling scope alongside the
// imported module's fragment.
namespace shared_ns
{
    int helper()
    {
        return 0;
    }
}

// The attribute lives in the imported fragment of `shared_ns`; resolving it requires searching the
// merged namespace's sibling scopes. A failed resolution would report 'shared_ns_Imported'.
// CHECK-NOT: shared_ns_Imported
[shared_ns::Imported(1)]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
