// Regression test for https://github.com/shader-slang/slang/issues/13701
//
// A qualified namespace declaration that reopens existing namespaces keeps the imported
// members of `Outer` visible to code that an earlier `namespace Outer` body declares, and
// it makes the members that this module declares in `Outer` visible inside its own body.
// Without the first, `ImportedType` in `Local` would be an undefined identifier; without
// the second, `outerValue` in `getOuterValue` would be.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import qualified_reopen_lib;

namespace Outer
{
static const int outerValue = 5;
namespace Inner
{
}
}

namespace Outer
{
struct Local
{
    // `ImportedType` is referenced unqualified; it resolves to the imported
    // `Outer::ImportedType`.
    ImportedType field;
}
}

// `Outer` and `Inner` both exist already.
namespace Outer::Inner
{
int getOuterValue()
{
    return outerValue;
}
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Outer::Local local;
    local.field.value = 3;
    outputBuffer[0] = local.field.value;
    outputBuffer[1] = Outer::Inner::getOuterValue();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}3{{$}}
// CHECK-NEXT: {{^}}5{{$}}
