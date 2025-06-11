//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// tests:
// loop without write
// if-else as write
// loop without write

import common;
RWBuffer<int> thing;
void logic(out some IFoo v1)
{
    // CHECK-NOT: error 41404
    for (int i = 0; i < thing[0]; i++)
    {
        thing[1] += 1;
    }

    if(buffer[0] == 0)
    {
        v1 = StructForIFoo1(1);
    }
    else if(buffer[0] == 1)
    {
        v1 = StructForIFoo1(2);
    }
    else
    {
        v1 = StructForIFoo1(3);
    }

    for (int i = 0; i < thing[0]; i++)
    {
        thing[2] += v1.getVal();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}