// Interface types cannot be used in varying (inter-stage) outputs because
// existential values carry runtime type information and witness tables that
// cannot be interpolated between pipeline stages.

//TEST:SIMPLE(filecheck=CHK1): -target spirv -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHK1): -target hlsl -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHK2): -target spirv -entry fragmentReturnMain -stage fragment
//TEST:SIMPLE(filecheck=CHK2): -target hlsl -entry fragmentReturnMain -stage fragment
//TEST:SIMPLE(filecheck=CHK3): -target spirv -entry fragmentOptionalReturn -stage fragment
//TEST:SIMPLE(filecheck=CHK3): -target hlsl -entry fragmentOptionalReturn -stage fragment

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
    // CHK1: error[E39028]
    // CHK1: parameter 'foo' is not a valid varying parameter
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
// CHK2: error[E39028]
// CHK2: parameter 'fragmentReturnMain' is not a valid varying parameter
{
    ImplA a;
    a.x = 42.0;
    return a;
}

// Scenario 3: Optional<IFoo> as entry point return value.
// The Optional decomposes into a tuple (IFoo, bool) internally.
[shader("fragment")]
Optional<IFoo> fragmentOptionalReturn() : SV_Target
// CHK3: error[E39028]
{
    ImplA a;
    a.x = 42.0;
    return Optional<IFoo>(a);
}
