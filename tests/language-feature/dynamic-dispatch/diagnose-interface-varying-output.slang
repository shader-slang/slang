// Interface types cannot be used in varying (inter-stage) outputs because
// existential values carry runtime type information and witness tables that
// cannot be interpolated between pipeline stages.
//
//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl
//TEST:SIMPLE(filecheck=CHECK): -target glsl
//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK): -target wgsl

interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float x;
    float getValue() { return x; }
}

// Scenario 1: Interface-typed field in vertex output struct.
struct VertexOutput
{
    IFoo foo : TEXCOORD0;
    // CHECK: error[E39028]
    // CHECK: parameter 'foo' is not a valid varying parameter
    float4 pos : SV_Position;
}

[shader("vertex")]
VertexOutput vertexMain(float3 position : POSITION)
{
    VertexOutput output;
    ImplA a;
    a.x = 1.0;
    output.foo = a;
    output.pos = float4(position, 1.0);
    return output;
}

// Scenario 2: Interface type as entry point return value.
[shader("fragment")]
IFoo fragmentReturnMain() : SV_Target
// CHECK: error[E39028]
// CHECK: parameter 'fragmentReturnMain' is not a valid varying parameter
{
    ImplA a;
    a.x = 42.0;
    return a;
}
