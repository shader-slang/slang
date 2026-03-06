// Interface types cannot be used in varying (inter-stage) inputs because
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

// Scenario 1: Interface-typed field in fragment input struct.
struct FragmentInput
{
    IFoo foo : TEXCOORD0;
    // CHECK: error[E39028]
    // CHECK: parameter 'foo' is not a valid varying parameter
    float4 pos : SV_Position;
}

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target
{
    return float4(input.foo.getValue(), 0, 0, 1);
}

// Scenario 2: Interface type as direct entry point varying parameter.
[shader("fragment")]
float4 fragmentDirectParam(IFoo foo : TEXCOORD0) : SV_Target
// CHECK: error[E39028]
// CHECK: parameter 'foo' is not a valid varying parameter
{
    return float4(foo.getValue(), 0, 0, 1);
}
