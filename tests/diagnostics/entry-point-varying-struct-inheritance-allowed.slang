// Verify that the struct-inheritance varying rule (issue #12463) does not
// reject uses that already compile correctly: HLSL emits the struct as
// written, and Metal and WGSL handle a derived struct as a varying output.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fragmentInput
//TEST:SIMPLE(filecheck=METAL): -target metal -entry vertexOutput
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry vertexOutput

struct Base
{
    float4 a : A;
};

struct Derived : Base
{
    float4 b : B;
};

[shader("fragment")]
float4 fragmentInput(Derived i) : SV_Target
{
    return i.a + i.b;
}

struct VertexOut : Base
{
    float4 pos : SV_Position;
};

[shader("vertex")]
VertexOut vertexOutput(float4 p : P)
{
    VertexOut o;
    o.a = p;
    o.pos = p;
    return o;
}

// HLSL-NOT: E38051
// HLSL: fragmentInput(
// METAL-NOT: E38051
// METAL: vertexOutput_Result
// WGSL-NOT: E38051
// WGSL: @vertex
