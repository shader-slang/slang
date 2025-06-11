//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// tests:
// if-else branch, nested, with 2 writes

import common;
void logic(out some IFoo v1)
{
    // CHECK: error 41404
    if(buffer[0] == 0)
    {
    }
    else if(buffer[0] == 1)
    {
        v1 = StructForIFoo1(1);
        if (buffer[1] == 2)
        {
            v1 = StructForIFoo1(2);    
        }
    }
    else
    {
        v1 = StructForIFoo1(3);
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    logic(data1);
    buffer[0] = data1.getVal();
}