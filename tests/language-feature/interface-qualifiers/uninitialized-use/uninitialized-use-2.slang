//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DUSE_DYN

// tests:
// uninitialized val assigned to 'in' function param

#ifdef USE_DYN
#define QUALIFIER dyn
#else
#define QUALIFIER some
#endif

import common;
void logic(in QUALIFIER IFoo v1)
{
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
// CHECK: error 41016
    QUALIFIER IFoo data1;
    logic(data1);
}