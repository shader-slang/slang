//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// tests:
// if-else branch with write
// simple write

import common;
void nestedLogic(out some IFoo v1)
{
    v1 = StructForIFoo1(1);
}
void logic(out some IFoo v1)
{
    // CHECK: error 41404
    if(buffer[0] == 0)
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