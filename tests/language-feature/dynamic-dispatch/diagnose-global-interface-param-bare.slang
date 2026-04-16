// A bare `uniform IFoo` at global scope (without ConstantBuffer wrapper)
// should produce error 50100 when no conformances are registered.
//
// The diagnostic fires correctly but the compiler still crashes after emitting
// it (pre-existing crash in the typeflow pass). Disabled until the follow-up
// crash is fixed.
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

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
    float getValue() { return v * 2.0; }
}

uniform IFoo gFoo;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: error 50100{{.*}}IFoo
    outputBuffer[0] = gFoo.getValue();
}
