// The fragment entry point is legalized before the vertex entry point that shares its struct, so
// stripping `invariant` from the fragment input must not take it off the vertex output.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl
//TEST:SIMPLE(filecheck=METAL): -target metal

struct Inner
{
    float2 uv : TEXCOORD0;
};

struct VSOut
{
    invariant float4 pos : SV_Position;
    Inner inner;
};

[shader("fragment")]
float4 fragmentMain(VSOut input) : SV_Target
{
    return float4(input.inner.uv, input.pos.z, 1.0);
}

[shader("vertex")]
VSOut vertexMain(float3 p : POSITION)
{
    VSOut output;
    output.pos = float4(p, 1.0);
    output.inner.uv = p.xy;
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
