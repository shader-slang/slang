//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry main -stage compute -DNO_PARAMETER_BLOCK

// An array of combined texture-samplers that is itself a field of a constant buffer or a struct,
// or the element of a parameter block, is split into a texture array and a sampler array, and
// each array keeps the binding that reflection reports for the combined array. The texture and
// sampler before them make every expected index non-zero.

Texture2D<float4> pre;
SamplerState preS;

struct M
{
    Sampler2D<float4> maps[2];
    Sampler2D<float4> extra;
    float4 k;
}
ConstantBuffer<M> cbm;

cbuffer CA
{
    Sampler2D<float4> carr[3];
    float4 ca;
}

#ifndef NO_PARAMETER_BLOCK
ParameterBlock<Sampler2D<float4>[4]> pba;
#endif

RWStructuredBuffer<float4> o;

// HLSL-DAG: Texture2D<float4 > cbm_maps_texture_0[int(2)] : register(t1);
// HLSL-DAG: SamplerState cbm_maps_sampler_0[int(2)] : register(s1);
// HLSL-DAG: Texture2D<float4 > cbm_extra_texture_0 : register(t3);
// HLSL-DAG: SamplerState cbm_extra_sampler_0 : register(s3);
// HLSL-DAG: Texture2D<float4 > CA_carr_texture_0[int(3)] : register(t4);
// HLSL-DAG: SamplerState CA_carr_sampler_0[int(3)] : register(s4);
// HLSL-DAG: Texture2D<float4 > pba_texture_0[int(4)] : register(t7);
// HLSL-DAG: SamplerState pba_sampler_0[int(4)] : register(s7);

// WGSL-DAG: @binding(3) @group(0) var cbm_maps_texture_0 :
// WGSL-DAG: @binding(4) @group(0) var cbm_maps_sampler_0 :
// WGSL-DAG: @binding(5) @group(0) var cbm_extra_texture_0 :
// WGSL-DAG: @binding(6) @group(0) var cbm_extra_sampler_0 :
// WGSL-DAG: @binding(8) @group(0) var CA_carr_texture_0 :
// WGSL-DAG: @binding(9) @group(0) var CA_carr_sampler_0 :
// WGSL-DAG: @binding(0) @group(1) var pba_texture_0 :
// WGSL-DAG: @binding(1) @group(1) var pba_sampler_0 :

// METAL-DAG: cbm_maps_texture_{{[0-9]+}} {{\[\[}}texture(1){{\]\]}}
// METAL-DAG: cbm_maps_sampler_{{[0-9]+}} {{\[\[}}sampler(1){{\]\]}}
// METAL-DAG: cbm_extra_texture_{{[0-9]+}} {{\[\[}}texture(3){{\]\]}}
// METAL-DAG: cbm_extra_sampler_{{[0-9]+}} {{\[\[}}sampler(3){{\]\]}}
// METAL-DAG: CA_carr_texture_{{[0-9]+}} {{\[\[}}texture(4){{\]\]}}
// METAL-DAG: CA_carr_sampler_{{[0-9]+}} {{\[\[}}sampler(4){{\]\]}}

[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    float2 uv = float2(id.xy);
    float4 r = pre.SampleLevel(preS, uv, 0);
    r += cbm.maps[id.x].SampleLevel(uv, 0) + cbm.maps[1].Load(int3(0)) + cbm.extra.SampleLevel(uv, 0)
        + cbm.k;
    r += carr[id.y].SampleLevel(uv, 0) + carr[2].Load(int3(0)) + ca;
#ifndef NO_PARAMETER_BLOCK
    r += pba[id.z].SampleLevel(uv, 0) + pba[3].Load(int3(0));
#endif
    o[0] = r;
}
