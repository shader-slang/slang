// A concrete struct that implements an interface can be used as a varying
// parameter because it is a fully resolved type with known layout. The
// compiler correctly distinguishes between interface-typed varying (invalid)
// and concrete-type-implementing-interface varying (valid).

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry vertexMain -stage vertex

interface IFoo
{
    float getValue();
}

struct FooImpl : IFoo
{
    float x;
    float getValue() { return x; }
}

struct VertexOutput
{
    FooImpl foo : TEXCOORD0;
    float4 pos : SV_Position;
}

// CHECK: vertexMain
[shader("vertex")]
VertexOutput vertexMain(float3 position : POSITION)
{
    VertexOutput output;
    output.foo.x = 1.0;
    output.pos = float4(position, 1.0);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return float4(input.foo.getValue(), 0, 0, 1);
}
