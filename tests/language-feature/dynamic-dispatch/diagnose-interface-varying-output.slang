// Interface types cannot be used in varying (inter-stage) outputs because
// existential values carry runtime type information and witness tables that
// cannot be interpolated between pipeline stages.
//
// Currently triggers an ICE ("unhandled type kind") in parameter binding
// instead of a clean diagnostic. Disabled until #10403 is fixed.

//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target hlsl
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target glsl
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target metal
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target wgsl

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
