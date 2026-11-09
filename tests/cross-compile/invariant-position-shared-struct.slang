// Both entry points are legalized together, fragment first. The fragment input nests the shared
// struct, so it gets flattened, and stripping `invariant` there must not reach the vertex output.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl
//TEST:SIMPLE(filecheck=METAL): -target metal

struct VSOut
{
    invariant float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

struct FragmentInput
{
    VSOut vertex;
};

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target
{
    return float4(input.vertex.uv, input.vertex.pos.z, 1.0);
}

[shader("vertex")]
VSOut vertexMain(float3 p : POSITION)
{
    VSOut output;
    output.pos = float4(p, 1.0);
    output.uv = p.xy;
    return output;
}

// WGSL-NOT: invariant
// WGSL: fn fragmentMain
// WGSL-NOT: invariant
// WGSL: @invariant @builtin(position)
// WGSL: fn vertexMain

// METAL-NOT: {{position, invariant}}
// METAL: {{\[\[fragment\]\]}}
// METAL: {{\[\[position, invariant\]\]}}
// METAL: {{\[\[vertex\]\]}}
