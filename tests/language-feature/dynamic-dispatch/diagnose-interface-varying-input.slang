// Interface types cannot be used in varying (inter-stage) inputs because
// existential values carry runtime type information and witness tables that
// cannot be interpolated between pipeline stages.

//TEST:SIMPLE(filecheck=CHK1): -target spirv -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=CHK1): -target hlsl -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=CHK2): -target spirv -entry fragmentDirectParam -stage fragment
//TEST:SIMPLE(filecheck=CHK2): -target hlsl -entry fragmentDirectParam -stage fragment
//TEST:SIMPLE(filecheck=CHK3): -target spirv -entry fragmentVkLocation -stage fragment

interface IFoo
{
    float getValue();
}

// Scenario 1: Interface-typed field in fragment input struct.
struct FragmentInput
{
    IFoo foo : TEXCOORD0;
    // CHK1: error[E39028]
    // CHK1: parameter 'foo' is not a valid varying parameter
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
// CHK2: error[E39028]
// CHK2: parameter 'foo' is not a valid varying parameter
{
    return float4(foo.getValue(), 0, 0, 1);
}

// Scenario 3: Interface-typed field with [[vk::location]] attribute.
// Before the fix, this would ICE because processEntryPointVaryingParameterDecl
// tried to call typeLayout->FindResourceInfo() on a null typeLayout.
struct FragmentInputVkLocation
{
    [[vk::location(0)]] IFoo foo;
    // CHK3: error[E39028]
    // CHK3: parameter 'foo' is not a valid varying parameter
    float4 pos : SV_Position;
}

[shader("fragment")]
float4 fragmentVkLocation(FragmentInputVkLocation input) : SV_Target
{
    return float4(input.foo.getValue(), 0, 0, 1);
}
