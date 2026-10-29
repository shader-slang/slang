//TEST:SIMPLE(filecheck=CPP): -target cpp -entry main -stage compute
//TEST:SIMPLE(filecheck=SO): -target shader-sharedlib -entry main -stage compute

// On CPU-like targets the lowered `{texture, sampler}` struct of a combined texture-sampler is not
// split into separate resources, so one nested in a constant buffer, a struct or the uniform
// parameters of an entry point stays a struct member. `lowerCombinedTextureSamplers` still
// rebuilds the layouts of these nested fields on CPU, where they carry `Uniform` sizes; this test
// guards that the walk's layout-kind assertions hold for the `globalParams` and
// `entryPointParams` structs and that the C++ reads the texture field of each member.

cbuffer C
{
    Sampler2D<float4> a;
    Sampler2D<float4> b;
    float4 c;
}

struct S
{
    Sampler2D<float4> t;
    float4 k;
}
ConstantBuffer<S> cb;
RWStructuredBuffer<float4> o;

// CPP-DAG: Sampler2D_0 b_0;
// CPP-DAG: Sampler2D_0 t_0;
// CPP-DAG: Sampler2D_0 ept_0;
// CPP: ->b_0.texture_0.SampleLevel(
// CPP-SAME: ->b_0.texture_0.Load(
// CPP-SAME: ->t_0.texture_0.Load(
// CPP-SAME: ->ept_0.texture_0.SampleLevel(
// SO: result code = 0
[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID, uniform Sampler2D<float4> ept)
{
    float2 uv = float2(id.xy);
    o[0] = b.SampleLevel(uv, 0) + b.Load(int3(id.xy, 0)) + cb.t.Load(int3(0))
        + ept.SampleLevel(uv, 0) + c + cb.k;
}
