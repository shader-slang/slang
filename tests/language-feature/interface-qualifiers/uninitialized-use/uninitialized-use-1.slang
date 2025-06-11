//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DUSE_DYN

// tests:
// uninitialized `out` assigned to `out`

#ifdef USE_DYN
#define QUALIFIER dyn
#else
#define QUALIFIER some
#endif

import common;
void logic(out QUALIFIER IFoo v1, out QUALIFIER IFoo v2)
{
// CHECK: error 41015
    v1 = v2;
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    QUALIFIER IFoo data1;
    QUALIFIER IFoo data2;
    logic(data1, data2);
    // buffer[0] = data1.getVal();
}