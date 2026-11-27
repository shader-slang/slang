//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage compute -target hlsl
// HLSL: void main()

// A familiar spelling without __magic_type is an ordinary interface. Its implementation may
// carry state, be constructed, and receive an ordinary call to invoke.
interface IMissShader
{
    uint invoke();
}

struct MissInput
{
    uint value;
}

struct OrdinaryStage : IMissShader
{
    MissInput input;
    uint invoke() { return input.value; }
}

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    OrdinaryStage stage = { { 7 } };
    output[0] = stage.invoke();
}
