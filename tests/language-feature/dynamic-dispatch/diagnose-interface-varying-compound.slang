// Compound types containing interface-typed members (nested structs, arrays,
// Optional) are also invalid as varying parameters. The existential data
// embedded inside these compound types cannot be interpolated.
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

// Scenario 1: Struct containing an interface field used as varying.
struct VaryingData
{
    float3 normal;
    IFoo material;
}

struct VertexOutputStruct
{
    VaryingData data : TEXCOORD0;
    // CHECK: error[E39028]
    // CHECK: parameter 'material' is not a valid varying parameter
    float4 pos : SV_Position;
}

[shader("vertex")]
VertexOutputStruct vertexStructMain(float3 position : POSITION)
{
    VertexOutputStruct output;
    ImplA a;
    a.x = 1.0;
    output.data.normal = float3(0, 1, 0);
    output.data.material = a;
    output.pos = float4(position, 1.0);
    return output;
}

// Scenario 2: Array of interface type as varying field.
struct VertexOutputArray
{
    IFoo foos[2] : TEXCOORD0;
    // CHECK: error[E39028]
    // CHECK: parameter 'foos' is not a valid varying parameter
    float4 pos : SV_Position;
}

[shader("vertex")]
VertexOutputArray vertexArrayMain(float3 position : POSITION)
{
    VertexOutputArray output;
    ImplA a, b;
    a.x = 1.0;
    b.x = 2.0;
    output.foos[0] = a;
    output.foos[1] = b;
    output.pos = float4(position, 1.0);
    return output;
}

// Scenario 3: Optional<IFoo> as varying field.
struct VertexOutputOptional
{
    Optional<IFoo> foo : TEXCOORD0;
    // CHECK: error[E39028]
    // CHECK: parameter 'foo' is not a valid varying parameter
    float4 pos : SV_Position;
}

[shader("vertex")]
VertexOutputOptional vertexOptionalMain(float3 position : POSITION)
{
    VertexOutputOptional output;
    ImplA a;
    a.x = 1.0;
    output.foo = Optional<IFoo>(a);
    output.pos = float4(position, 1.0);
    return output;
}
