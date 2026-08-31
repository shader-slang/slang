// SampleLevel of a half-typed texture on the CUDA target must be rejected with a
// source-level error (E41400): the CUDA `tex*Lod<T>` built-ins have no __half
// overload, so a half texel would otherwise emit CUDA that NVRTC cannot compile.
// float/uint/int textures must still emit a `tex*Lod<...>` call, and a generic
// float SampleLevel wrapper must compile with no diagnostic (the guard is a
// per-specialization type predicate, so it must not fire for a symbolic T).

// SamplerState-argument form: tex.SampleLevel(sampler, location, level)
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H1): -target cuda -entry eHalf -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H2): -target cuda -entry eHalf2 -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H3): -target cuda -entry eHalf3 -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H4): -target cuda -entry eHalf4 -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H3D): -target cuda -entry eHalf4_3D -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H2DA): -target cuda -entry eHalf4_2DArr -stage compute

// Combined texture-sampler form: sampler2D.SampleLevel(location, level)
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HC): -target cuda -entry eHalfCombined -stage compute

// Supported texel types emit a clean call, with no diagnostic.
//TEST:SIMPLE(filecheck=FLOAT): -target cuda -entry eFloat -stage compute
//TEST:SIMPLE(filecheck=UINT): -target cuda -entry eUint -stage compute
//TEST:SIMPLE(filecheck=INT): -target cuda -entry eInt -stage compute
//TEST:SIMPLE(filecheck=FLOAT2DA): -target cuda -entry eFloat4_2DArr -stage compute
//TEST:SIMPLE(filecheck=FLOATC): -target cuda -entry eFloatCombined -stage compute

// A generic float SampleLevel wrapper must compile with no diagnostic at all
// (an exhaustive DIAGNOSTIC_TEST with no annotations passes iff zero diagnostics
// are emitted), proving the guard does not fire for a symbolic T. A separate
// code-generation check confirms the concrete float specialization still emits.
//DIAGNOSTIC_TEST:SIMPLE(diag=GEN): -target cuda -entry eGenericFloat -stage compute
//TEST:SIMPLE(filecheck=GENERIC): -target cuda -entry eGenericFloat -stage compute

Texture2D<half>        gHalf;
Texture2D<half2>       gHalf2;
Texture2D<half3>       gHalf3;
Texture2D<half4>       gHalf4;
Texture3D<half4>       gHalf4_3D;
Texture2DArray<half4>  gHalf4_2DArr;
Texture2D<float4>      gFloat4;
Texture2D<uint4>       gUint4;
Texture2D<int4>        gInt4;
Texture2DArray<float4> gFloat4_2DArr;

Sampler2D<half4>       gSamplerHalf4;
Sampler2D<float4>      gSamplerFloat4;

SamplerState samp;
RWStructuredBuffer<float> outBuf;

//H1: error[E41400]: static assertion failed
//H1: 'SampleLevel' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf() { outBuf[0] = gHalf.SampleLevel(samp, float2(0.5f), 0.0f); }

//H2: error[E41400]: static assertion failed
//H2: 'SampleLevel' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf2() { outBuf[0] = gHalf2.SampleLevel(samp, float2(0.5f), 0.0f).x; }

//H3: error[E41400]: static assertion failed
//H3: 'SampleLevel' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf3() { outBuf[0] = gHalf3.SampleLevel(samp, float2(0.5f), 0.0f).x; }

//H4: error[E41400]: static assertion failed
//H4: 'SampleLevel' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4() { outBuf[0] = gHalf4.SampleLevel(samp, float2(0.5f), 0.0f).x; }

//H3D: error[E41400]: static assertion failed
//H3D: 'SampleLevel' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4_3D() { outBuf[0] = gHalf4_3D.SampleLevel(samp, float3(0.5f), 0.0f).x; }

// The guard sits above the shape switch, so the layered (array) arm is rejected too.
//H2DA: error[E41400]: static assertion failed
//H2DA: 'SampleLevel' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4_2DArr() { outBuf[0] = gHalf4_2DArr.SampleLevel(samp, float3(0.5f), 0.0f).x; }

//HC: error[E41400]: static assertion failed
//HC: 'SampleLevel' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalfCombined() { outBuf[0] = gSamplerHalf4.SampleLevel(float2(0.5f), 0.0f).x; }

//FLOAT: tex2DLod<float4
//FLOAT-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eFloat() { outBuf[0] = gFloat4.SampleLevel(samp, float2(0.5f), 0.0f).x; }

//UINT: tex2DLod<uint4
//UINT-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eUint() { outBuf[0] = float(gUint4.SampleLevel(samp, float2(0.5f), 0.0f).x); }

//INT: tex2DLod<int4
//INT-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eInt() { outBuf[0] = float(gInt4.SampleLevel(samp, float2(0.5f), 0.0f).x); }

// Positive control for the array (layered) arm: a supported texel type still emits cleanly.
//FLOAT2DA: tex2DLayeredLod<float4
//FLOAT2DA-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eFloat4_2DArr() { outBuf[0] = gFloat4_2DArr.SampleLevel(samp, float3(0.5f), 0.0f).x; }

// Positive control for the combined texture-sampler form.
//FLOATC: tex2DLod<float4
//FLOATC-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eFloatCombined() { outBuf[0] = gSamplerFloat4.SampleLevel(float2(0.5f), 0.0f).x; }

// Regression guard for #12185: a generic float SampleLevel wrapper must compile cleanly.
// The guard is a `static_assert(!__isHalf<T>())` over a type predicate, which folds to a
// concrete bool per specialization (float here), so it must NOT fire at generic type-check.
T sampleR<T : ITexelElement>(Texture2D<T> t, SamplerState s)
{
    return t.SampleLevel(s, float2(0.5f), 0.0f);
}
//GENERIC: tex2DLod<float4
//GENERIC-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eGenericFloat() { outBuf[0] = sampleR<float4>(gFloat4, samp).x; }
