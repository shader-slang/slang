//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DUSE_DYN

// tests:
// returning uninitialized val

#ifdef USE_DYN
#define QUALIFIER dyn
#else
#define QUALIFIER some
#endif

import common;
QUALIFIER IFoo logic()
{
    // CHECK: error 41016
    QUALIFIER IFoo v1;
    return v1;
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    QUALIFIER IFoo data1 = logic();
    // buffer[0] = data1.getVal();
}