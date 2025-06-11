//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -lang slang -std 2026

// ensure we have a positive test for `some` in parameters

// CHECK: OpEntryPoint
// CHECK-NOT: error

interface IFoo
{
    int getVal();
}

struct myS : IFoo
{
    int v2;
    int getVal()
    {
        return 1;
    }
}

some IFoo test1(in some IFoo some1, in some IFoo some2, in some IFoo some3, int* myPtr)
{
    some IFoo some4 = myS(1);
    return some4;
}

func test2(some1: some IFoo, some2: some IFoo) -> some IFoo
{
    return myS(1);
}

RWStructuredBuffer<int> buffer;
void computeMain()
{
    some IFoo some2;
    some IFoo some3;
    var some4 : some IFoo;
    let some5 : some IFoo;
}