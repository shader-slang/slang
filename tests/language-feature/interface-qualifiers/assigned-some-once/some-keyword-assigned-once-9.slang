//IGNORE:TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// Note: this fails since we cannot unroll these loops (Slang limitation?)

import common;
RWBuffer<int> thing;
void logic(out some IFoo v1)
{
    // CHECK-NOT: error 41404
    [ForceUnroll]
    for (int i = 0; i < 1; i++)
    {
        v1 = StructForIFoo1(1);
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}