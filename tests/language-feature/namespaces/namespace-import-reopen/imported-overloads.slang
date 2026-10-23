// Regression test for https://github.com/shader-slang/slang/issues/11442
//
// Every body of a reopened namespace has its own link to the imported namespace, and lookup
// of a function keeps searching outward after a hit to collect the whole overload set. A
// call to an imported overload from any body, a nested namespace or a qualified reopen
// therefore resolves to one overload by argument type and is not reported as ambiguous.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import reopen_lib;

namespace Foo
{
int a()
{
    return over(1);
}

namespace Nested
{
int b()
{
    return over(1.0f);
}
}
}

namespace Foo
{
int c()
{
    return over(1);
}
}

namespace Foo
{
namespace Nested
{
int d()
{
    return over(1);
}
}
}

// `Foo` and `Nested` both exist already.
namespace Foo::Nested
{
int e()
{
    return over(1.0f);
}
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = Foo::a();
    outputBuffer[1] = Foo::Nested::b();
    outputBuffer[2] = Foo::c();
    outputBuffer[3] = Foo::Nested::d();
    outputBuffer[4] = Foo::Nested::e();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}2{{$}}
