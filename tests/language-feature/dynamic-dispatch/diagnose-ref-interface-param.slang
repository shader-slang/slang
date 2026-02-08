//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that __ref and __constref parameters with interface types
// in dynamic dispatch contexts produce diagnostic errors.
//
// Dynamic dispatch uses a tagged union (AnyValue) representation
// which is incompatible with reference semantics.

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2; }
}

// Scenario 1: Direct __ref interface parameter
// CHECK: ([[# @LINE+1]]): error 52010: '__ref' parameter of type 'IFoo' cannot be used in dynamic dispatch context.
float processRef(__ref IFoo obj)
{
    return obj.getValue();
}

// Scenario 2: Direct __constref interface parameter
// CHECK: ([[# @LINE+1]]): error 52010: '__constref' parameter of type 'IFoo' cannot be used in dynamic dispatch context.
float processConstref(__constref IFoo obj)
{
    return obj.getValue();
}

IFoo factory(uint id)
{
    if (id == 0) return ImplA(1.0);
    return ImplB(2.0);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IFoo obj = factory(dispatchThreadID.x);
    outputBuffer[0] = processRef(obj);
    outputBuffer[1] = processConstref(obj);
}
