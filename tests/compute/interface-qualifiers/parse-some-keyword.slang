//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

//CHECK-NOT: error

interface IFoo
{
}

struct myS : IFoo
{
    int v2;
}

some IFoo test1(in some IFoo some1, out some IFoo some2, inout some IFoo some3, int* myPtr)
{
    some IFoo some4 = myS();
    return some4;
}

func test2(some1: some IFoo, some2: some IFoo) -> some IFoo
{
    return myS();
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some some1 : IFoo;
    some IFoo some2;
    some IFoo some3;
    buffer[0] = test1(some1, some2, some3) + test2();
}