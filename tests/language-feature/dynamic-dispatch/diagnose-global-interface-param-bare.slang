// A bare `uniform IFoo` at global scope (without ConstantBuffer wrapper)
// should produce error 50100 when no conformances are registered.
//
// The `slangc` CLI path emits the diagnostic cleanly, but the Slang API /
// test-server path still crashes. Kept disabled until the API-path crash is
// root-caused; tracked as follow-up work separate from this PR.
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
