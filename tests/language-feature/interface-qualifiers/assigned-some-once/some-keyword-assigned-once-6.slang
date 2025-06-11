//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// tests:
// loop with write

import common;
RWBuffer<int> thing;
void logic(out some IFoo v1)
{
    // CHECK: error 41404
    for (int i = 0; i < thing[0]; i++)
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