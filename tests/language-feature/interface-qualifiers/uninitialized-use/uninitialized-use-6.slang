//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DUSE_DYN

// tests:
// returning initialized val

// CHECK: OpEntryPoint
// CHECK-NOT: error

#ifdef USE_DYN
#define QUALIFIER dyn
#else
#define QUALIFIER some
#endif

import common;
QUALIFIER IFoo logic()
{
    QUALIFIER IFoo v1 = StructForIFoo1(1);
    return v1;
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    QUALIFIER IFoo data1 = logic();
    buffer[0] = data1.getVal();
}