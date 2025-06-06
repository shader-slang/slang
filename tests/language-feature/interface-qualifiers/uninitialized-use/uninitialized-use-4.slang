//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DUSE_DYN

// tests:
// uninitialized val assigned to 'out' function param

#ifdef USE_DYN
#define QUALIFIER dyn
#else
#define QUALIFIER some
#endif

import common;
// CHECK: error 41018
void logic(out QUALIFIER IFoo v1)
{
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    QUALIFIER IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}