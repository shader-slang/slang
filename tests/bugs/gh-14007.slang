//TEST:SIMPLE(filecheck=LOAD): -target metal -entry loadMain -stage fragment
//TEST:SIMPLE(filecheck=DIMS): -target metal -entry dimsMain -stage fragment
//TEST:SIMPLE(filecheck=SCALAR): -target metal -entry scalarMain -stage fragment
//TEST:SIMPLE(filecheck=SUBSCRIPT): -target metal -entry subscriptMain -stage fragment
//TEST:SIMPLE(filecheck=LOADONLY): -target metal -entry loadOnlyMain -stage fragment
//TEST:SIMPLE(filecheck=SCALARLOADONLY): -target metal -entry scalarLoadOnlyMain -stage fragment
//TEST:SIMPLE(filecheck=FIELD): -target metal -entry fieldMain -stage fragment
//TEST:SIMPLE(filecheck=MS): -target metal -entry msMain -stage fragment
//TEST:SIMPLE(filecheck=HELPER): -target metal -entry helperMain -stage fragment
//TEST:SIMPLE(filecheck=COMPUTE): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HANDLE): -target metal -entry handleMain -stage fragment
//TEST:SIMPLE(filecheck=NESTED): -target metal -entry nestedMain -stage compute
// The Metal compiler, where it is available, must accept the argument-buffer accesses.
//TEST:SIMPLE(filecheck=LIBLOAD): -target metallib -entry loadMain -stage fragment
//TEST:SIMPLE(filecheck=LIBDIMS): -target metallib -entry dimsMain -stage fragment
//TEST:SIMPLE(filecheck=LIBSCALAR): -target metallib -entry scalarLoadOnlyMain -stage fragment
//TEST:SIMPLE(filecheck=LIBFIELD): -target metallib -entry fieldMain -stage fragment
//TEST:SIMPLE(filecheck=LIBMS): -target metallib -entry msMain -stage fragment

// shader-slang/slang#14007 and #14006: on Metal, a combined texture-sampler inside a
// `ParameterBlock` is an argument-buffer field of the lowered `{texture_0, sampler_0}` struct.
// Unlike a plain global, that struct is never split into separate call arguments, so the
// texture-only operations (`Load`, the subscripts and `GetDimensions`) must read its `texture_0`
// member, the same way `Sample` does. A block that only uses those operations must still hold
// both members, because reflection lays out `Sampler2D` as a texture and a sampler.

ParameterBlock<Sampler2D[16]> gArray;
ParameterBlock<Sampler2D> gScalar;

// LIBLOAD: @loadMain
// LOAD: texture_0).sample(
// LOAD-SAME: texture_0).read(
[shader("fragment")]
float4 loadMain(float2 uv : TEXCOORD0, nointerpolation uint i : TEXCOORD1) : SV_Target
{
    return gArray[i].SampleLevel(uv, 0) + gArray[i + 1].Load(int3(1, 1, 0));
}

// LIBDIMS: @dimsMain
// The dimension queries take the extracted texture, not the address of the block element.
// DIMS: texture2d<float, access::sample> [[T0:_S[0-9]+]] = gArray_0->data_0[{{[_A-Za-z0-9.]+}}].texture_0;
// DIMS: = ([[T0]]).get_width(0)),{{.*}} = ([[T0]]).get_height(0));
// DIMS: texture2d<float, access::sample> [[T1:_S[0-9]+]] = gArray_0->data_0[{{[_A-Za-z0-9.]+}}].texture_0;
// DIMS: = ([[T1]]).get_width((1U))),{{.*}} = ([[T1]]).get_num_mip_levels());
[shader("fragment")]
float4 dimsMain(float2 uv : TEXCOORD0, nointerpolation uint i : TEXCOORD1) : SV_Target
{
    uint w, h, levels;
    gArray[i].GetDimensions(w, h);
    float4 r = float4(w, h, 0, 1);
    gArray[i].GetDimensions(1, w, h, levels);
    r += float4(w, h, levels, 0);
    return gArray[i].SampleLevel(uv, 0) + r;
}

// SCALAR: texture_0).sample(
// SCALAR-SAME: texture_0).read(
[shader("fragment")]
float4 scalarMain(float2 uv : TEXCOORD0) : SV_Target
{
    return gScalar.Sample(uv) + gScalar.Load(int3(0, 0, 0));
}

// SUBSCRIPT: texture_0).sample(
// SUBSCRIPT-SAME: texture_0).read(
[shader("fragment")]
float4 subscriptMain(float2 uv : TEXCOORD0, nointerpolation uint i : TEXCOORD1) : SV_Target
{
    return gArray[i].SampleLevel(uv, 0) + gArray[i][uint2(1, 1)];
}

ParameterBlock<Sampler2D[16]> gLoadOnly;

// LOADONLY: struct [[ELEM:Sampler2D[A-Za-z0-9_]*]]
// LOADONLY-NEXT: {
// LOADONLY-NEXT: texture2d<float, access::sample> texture_0;
// LOADONLY-NEXT: sampler sampler_0;
// LOADONLY: array<[[ELEM]], int(16)> {{.*}}
// LOADONLY: texture2d<float, access::sample> [[T:_S[0-9]+]] = gLoadOnly_0->data_0[{{[_A-Za-z0-9.]+}}].texture_0;
// LOADONLY: = ([[T]]).get_width(0)),
// LOADONLY: texture_0).read(
// LOADONLY-SAME: texture_0).read(
[shader("fragment")]
float4 loadOnlyMain(nointerpolation uint i : TEXCOORD1) : SV_Target
{
    uint w, h;
    gLoadOnly[i].GetDimensions(w, h);
    return gLoadOnly[i].Load(int3(1, 1, 0)) + gLoadOnly[i][uint2(2, 2)] + float4(w, h, 0, 1);
}

ParameterBlock<Sampler2D> gScalarLoadOnly;

// LIBSCALAR: @scalarLoadOnlyMain
// SCALARLOADONLY: struct [[ELEM:Sampler2D[A-Za-z0-9_]*]]
// SCALARLOADONLY-NEXT: {
// SCALARLOADONLY-NEXT: texture2d<float, access::sample> texture_0;
// SCALARLOADONLY-NEXT: sampler sampler_0;
// SCALARLOADONLY: [[ELEM]] constant* gScalarLoadOnly_0
// SCALARLOADONLY: (*gScalarLoadOnly_0).texture_0).read(
// SCALARLOADONLY-SAME: (*gScalarLoadOnly_0).texture_0).read(
[shader("fragment")]
float4 scalarLoadOnlyMain(nointerpolation uint i : TEXCOORD1) : SV_Target
{
    return gScalarLoadOnly.Load(int3(i, 1, 0)) + gScalarLoadOnly[uint2(2, i)];
}

struct Material
{
    Sampler2D tex;
    float4 tint;
};
ParameterBlock<Material> gMaterial;

// LIBFIELD: @fieldMain
// FIELD: texture2d<float, access::sample> [[T:_S[0-9]+]] = gMaterial_{{[0-9]+}}->tex_0.texture_0;
// FIELD: = ([[T]]).get_width(0)),
// FIELD: texture_0).sample(
// FIELD-SAME: texture_0).read(
[shader("fragment")]
float4 fieldMain(float2 uv : TEXCOORD0) : SV_Target
{
    uint w, h;
    gMaterial.tex.GetDimensions(w, h);
    return gMaterial.tex.Sample(uv) + gMaterial.tex.Load(int3(1, 1, 0)) + gMaterial.tint
        + float4(w, h, 0, 0);
}

ParameterBlock<Sampler2DMS> gMS;

// LIBMS: @msMain
// MS: struct [[ELEM:Sampler2DMS[A-Za-z0-9_]*]]
// MS-NEXT: {
// MS-NEXT: texture2d_ms<float, access::read> texture_0;
// MS-NEXT: sampler sampler_0;
// MS: [[ELEM]] constant* gMS_0
// MS: (*gMS_0).texture_0).read(
[shader("fragment")]
float4 msMain(nointerpolation int2 p : TEXCOORD1) : SV_Target
{
    return gMS.Load(p, 1);
}

ParameterBlock<Sampler2D> gHelper;

float4 sampleAndLoad(Sampler2D s, float2 uv)
{
    return s.Sample(uv) + s.Load(int3(0, 0, 0));
}

// HELPER: texture_0).sample(
// HELPER-SAME: texture_0).read(
[shader("fragment")]
float4 helperMain(float2 uv : TEXCOORD0) : SV_Target
{
    return sampleAndLoad(gHelper, uv);
}

ParameterBlock<Sampler2D> gCompute;
RWStructuredBuffer<float4> outBuf;

// COMPUTE: texture_0).sample(
// COMPUTE-SAME: texture_0).read(
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = gCompute.SampleLevel(float2(0.5), 0) + gCompute.Load(int3(tid.xy, 0));
}

// A bindless handle reaches `Load` as a `{texture_0, sampler_0}` value built from the handle.
StructuredBuffer<Sampler2D.Handle> gHandles;

// HANDLE: texture_0).sample(
// HANDLE-SAME: texture_0).read(
[shader("fragment")]
float4 handleMain(float2 uv : TEXCOORD0, nointerpolation uint i : TEXCOORD1) : SV_Target
{
    return gHandles[i].Sample(uv) + gHandles[i].Load(int3(1, 1, 0));
}

// Outside an argument buffer the lowered struct is split as before, and each texture keeps the
// binding that reflection reports, even when no sampling operation uses the texture-sampler.
cbuffer NestedCB
{
    Sampler2D<float4> nestedA;
    Sampler2D<float4> nestedB;
    float4 nestedC;
}
ConstantBuffer<Material> gNestedMaterial;
uniform Material gUniformMaterial;
RWStructuredBuffer<float4> nestedOut;

// NESTED-DAG: NestedCB_nestedB_texture_{{[0-9]+}} {{\[\[}}texture(1){{\]\]}}
// NESTED-DAG: gNestedMaterial_tex_texture_{{[0-9]+}} {{\[\[}}texture(2){{\]\]}}
// NESTED-DAG: globalParams_gUniformMaterial_tex_texture_{{[0-9]+}} {{\[\[}}texture(3){{\]\]}}
// NESTED-DAG: entryPointParams_epTex_texture_{{[0-9]+}} {{\[\[}}texture(4){{\]\]}}
// NESTED-DAG: (entryPointParams_epTex_texture_{{[0-9]+}}).get_width(0)
[shader("compute")]
[numthreads(1, 1, 1)]
void nestedMain(uint3 id: SV_DispatchThreadID, uniform Sampler2D<float4> epTex)
{
    uint w, h;
    epTex.GetDimensions(w, h);
    nestedOut[0] = nestedB.Load(int3(id.xy, 0)) + gNestedMaterial.tex[id.xy]
        + gUniformMaterial.tex.Load(int3(0)) + float4(w, h, 0, 0) + nestedC;
}
