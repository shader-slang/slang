// Regression test for https://github.com/shader-slang/slang/issues/11442
//
// Code in a body of a reopened namespace finds the imported members of that namespace when
// the body also declares a nested namespace, and so does code in the nested namespace.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import reopen_lib;

namespace Foo
{
struct Local
{
    ImportedType field;
}

namespace Nested
{
struct Inner
{
    ImportedType nestedField;
}
}
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo::Local local;
    local.field.value = 1;
    Foo::Nested::Inner inner;
    inner.nestedField.value = 2;
    outputBuffer[0] = local.field.value;
    outputBuffer[1] = inner.nestedField.value;
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}2{{$}}
