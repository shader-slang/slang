// Regression test for https://github.com/shader-slang/slang/issues/11442
//
// An imported member of a namespace hides a global of the same name in every body of a
// namespace that this module reopens, as a member that this module declares in the
// namespace does. Both `earlier` and `later` find the imported `Foo::importedValue`, not the
// global.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import reopen_lib;

static const int importedValue = 100;

namespace Foo
{
int earlier()
{
    return importedValue;
}
}

namespace Foo
{
int later()
{
    return importedValue;
}
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = Foo::earlier();
    outputBuffer[1] = Foo::later();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}7{{$}}
// CHECK-NEXT: {{^}}7{{$}}
