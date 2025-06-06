//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// What this tests:
// Test multiple cases happening at once and that multiple supporting erors are emitted

import common;
some IFoo nestedReturnLogic()
{
    return StructForIFoo1(1);
}

some IFoo nestedOutLogic(out some IFoo v1)
{
    v1 = StructForIFoo1(1);
}

// CHECK: error 33093
// CHECK: note: see use site
// CHECK: error 33093
// CHECK: note: see use site
// CHECK: error 33093
// CHECK: note: see use site
some IFoo logic(in some IFoo v2)
{
    if(buffer[0] == 0)
    {
        return nestedReturnLogic();
    }
    else if (buffer[0] == 1)
    {
        return v2;
    }
    else if (buffer[0] == 2)
    {
        return StructForIFoo1(2);
    }
    else
    {
        some IFoo v3;
        nestedOutLogic(v3);
        return v3;
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data2 = StructForIFoo1(2);
    some IFoo data1 = logic(data2);
    
    buffer[0] = data1.getVal();
}