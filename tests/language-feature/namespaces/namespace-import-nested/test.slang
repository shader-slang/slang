//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for https://github.com/shader-slang/slang/issues/11442
//
// Unqualified lookup of an imported type from inside a reopened namespace must
// still succeed when the reopened namespace ALSO contains a nested namespace.
// Previously the nested `namespace Nested {}` caused the parser to overwrite
// the owned scope of the reopened `Foo`, so cross-module sibling-scope wiring
// landed on a scope the use-site never walked, producing
// `error 30015: undefined identifier 'ImportedType'`.

import nested_namespace_lib;

namespace Foo
{
    struct Local
    {
        // `ImportedType` is referenced unqualified; it must resolve to the
        // imported `Foo::ImportedType`.
        ImportedType field;
    }

    // The mere presence of this nested namespace used to break the unqualified
    // lookup above.
    namespace Nested {}
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    Foo.Local local;
    local.field.value = tid + 1;
    outputBuffer[tid] = local.field.value;
    // CHECK: 1
    // CHECK: 2
    // CHECK: 3
    // CHECK: 4
}
