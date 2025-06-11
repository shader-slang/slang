//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// tests:
// constexpr resolve if, no write
// simple write

import common;
RWBuffer<int> thing;
void logic(out some IFoo v1)
{
    // CHECK-NOT: error 41404
    if(1 == 0)
    {
        v1 = StructForIFoo1(1);
    }
    v1 = StructForIFoo1(1);
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}