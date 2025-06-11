//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

import common;
// CHECK-NOT: error 33093
some IFoo logic()
{
    if(buffer[0] == 0)
    {
        return StructForIFoo1(1);
    }
    else
    {
        return StructForIFoo1(2); 
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1 = logic();
    buffer[0] = data1.getVal();
}