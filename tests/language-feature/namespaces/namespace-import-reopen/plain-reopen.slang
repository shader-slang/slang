// Regression test for https://github.com/shader-slang/slang/issues/11442
//
// Code in every body of a namespace that this module reopens finds the imported members of
// that namespace, although each `namespace Foo { ... }` body has its own scope.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import reopen_lib;

namespace Foo
{
struct Local
{
    ImportedType field;
}
}

namespace Foo
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
    outputBuffer[1] = Foo::getImportedValue();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}7{{$}}
