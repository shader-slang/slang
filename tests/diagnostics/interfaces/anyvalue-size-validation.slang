// anyvalue-size-validation.slang

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -conformance "A:IInterface=0" -conformance "B:IInterface=1"

[anyValueSize(8)]
interface IInterface
{
    int doSomething();
};

//CHECK: ([[# @LINE+2]]): error 41011: type 'A' does not fit in the size required by its conforming interface.
//CHECK-NEXT: struct A : IInterface
struct A : IInterface
{
    uint a;
    uint b;
    uint c;
    int doSomething() { return 5; }
};

//CHECK: note: sizeof(A) is 12, limit is 8

struct B : IInterface
{
    int doSomething() { return 6; }
};

//TEST_INPUT: 
//TEST_INPUT: type_conformance B:IInterface = 1

RWStructuredBuffer<IInterface> data;
RWStructuredBuffer<int> output;

struct Data
{
    float x;
    float y;
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    output[1] = data[0].doSomething();
}
