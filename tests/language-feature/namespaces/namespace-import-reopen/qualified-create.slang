// Regression test for https://github.com/shader-slang/slang/issues/11442
//
// A qualified declaration `namespace Foo::NewB { ... }` that creates `NewB` pushes a new
// body scope for `Foo`. Code in the earlier `Foo` body finds the imported members of `Foo`,
// and so does the body of `NewB`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import reopen_lib;

namespace Foo
{
struct Local
{
    ImportedType field;
}
}

// `Foo` exists already, and `NewB` is new.
namespace Foo::NewB
{
int getImportedValue()
{
    return importedValue;
}
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo::Local local;
    local.field.value = 1;
    outputBuffer[0] = local.field.value;
    outputBuffer[1] = Foo::NewB::getImportedValue();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}7{{$}}
