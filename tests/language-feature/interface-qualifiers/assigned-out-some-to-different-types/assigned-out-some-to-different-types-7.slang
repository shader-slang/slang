//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// What this tests:
// This mix of things causes Slang specialization system issues

import common;
some IFoo nestedReturnLogic()
{
    return StructForIFoo1(1);
}

some IFoo nestedOutLogic1(out some IFoo v1)
{
    v1 = StructForIFoo1(1);
}

__generic<T>
some IFoo nestedOutLogic2(out some IFoo v1)
{
    v1 = StructForIFoo1(1);
}


// CHECK: error 33094
void logic(out some IFoo v1, in some IFoo v2)
{
    if(buffer[0] == 0)
    {
        v1 = v2;
    }
    else if (buffer[0] == 1)
    {
        v1 = StructForIFoo1(2);
    }
    else if (buffer[0] == 2)
    {
        v1 = nestedReturnLogic();
    }
    else if (buffer[0] == 3)
    {
        nestedOutLogic2<int>(v1);
    }
    else if (buffer[0] == 4)
    {
        nestedOutLogic2<int>(v1);
    }
    else
    {
        nestedOutLogic1(v1);
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data1;
    some IFoo data2 = StructForIFoo1(2);
    logic(data1, data2);
    buffer[0] = data1.getVal();
}