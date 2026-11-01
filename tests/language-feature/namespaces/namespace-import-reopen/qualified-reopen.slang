// Regression test for https://github.com/shader-slang/slang/issues/13714
//
// Inside `namespace Foo::B { ... }`, where both `Foo` and `B` exist already, unqualified
// lookup finds the imported members of `Foo` through the lookup-only scope that the parser
// pushes for `Foo`, as it does inside `namespace Foo { namespace B { ... } }`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import reopen_lib;

namespace Foo
{
namespace B
{
}
}

// `Foo` and `B` both exist already.
namespace Foo::B
{
int getImportedValue()
{
    return importedValue;
}
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = Foo::B::getImportedValue();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}7{{$}}
