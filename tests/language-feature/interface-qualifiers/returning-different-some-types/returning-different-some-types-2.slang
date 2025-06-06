//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

import common;
// CHECK: error 33093
some IFoo logic(in some IFoo v2)
{
    if(buffer[0] == 0)
    {
        return StructForIFoo1(1);
    }
    else
    {
        return v2; // error since all `some` are different type
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data2 = StructForIFoo1(2);
    some IFoo data1 = logic(data2);
    buffer[0] = data1.getVal();
}