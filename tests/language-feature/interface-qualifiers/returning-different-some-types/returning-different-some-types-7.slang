//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// What this tests:
// Test returning 2 "equal" function. This is not-allowed
// since all functions must return the same type. Note that
// every `some` type is different (and the function return is a `some`) 

import common;
some IFoo nestedReturnLogic1()
{
    return StructForIFoo1(1);
}

some IFoo nestedReturnLogic2()
{
    return StructForIFoo1(3);
}

// CHECK: error 33093
some IFoo logic(in some IFoo v2)
{
    if (buffer[0] == 0)
    {
        return nestedReturnLogic1();
    }
    else
    {
        return nestedReturnLogic2();
    }
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo data2 = StructForIFoo1(2);
    some IFoo data1 = logic(data2);
    
    buffer[0] = data1.getVal();
}