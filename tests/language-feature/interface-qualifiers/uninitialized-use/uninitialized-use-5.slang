//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026 -DUSE_DYN

// tests:
// accessing member of uninitialized variable

#ifdef USE_DYN
#define QUALIFIER dyn
#else
#define QUALIFIER some
#endif

import common;
RWStructuredBuffer<int> buffer;
void computeMain()
{
    QUALIFIER IFoo data1;
// CHECK: error 41016
    buffer[0] = data1.getVal();
}