// SampleGrad of a half-typed texture on the CUDA target must be rejected with a
// source-level error (E41400): the CUDA `tex*Grad<T>` built-ins have no __half
// overload, so a half texel would otherwise emit CUDA that NVRTC cannot compile.
// float/uint/int textures must still emit a `tex*Grad<...>` call, and a generic
// float SampleGrad wrapper must compile with no diagnostic.

// SamplerState-argument form: tex.SampleGrad(sampler, location, gradX, gradY)
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H1): -target cuda -entry eHalf -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H4): -target cuda -entry eHalf4 -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H3D): -target cuda -entry eHalf4_3D -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H2DA): -target cuda -entry eHalf4_2DArr -stage compute

// Combined texture-sampler form: sampler2D.SampleGrad(location, gradX, gradY)
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HC): -target cuda -entry eHalfCombined -stage compute

// Supported texel types emit a clean call, with no diagnostic.
//TEST:SIMPLE(filecheck=FLOAT): -target cuda -entry eFloat -stage compute
//TEST:SIMPLE(filecheck=UINT): -target cuda -entry eUint -stage compute
//TEST:SIMPLE(filecheck=INT): -target cuda -entry eInt -stage compute
//TEST:SIMPLE(filecheck=FLOAT2DA): -target cuda -entry eFloat4_2DArr -stage compute
//TEST:SIMPLE(filecheck=FLOATC): -target cuda -entry eFloatCombined -stage compute

// A generic float SampleGrad wrapper must compile with no diagnostic at all (an
// exhaustive DIAGNOSTIC_TEST with no annotations passes iff zero diagnostics are
// emitted), so the guard does not fire for a symbolic T.
//DIAGNOSTIC_TEST:SIMPLE(diag=GEN): -target cuda -entry eGenericFloat -stage compute
//TEST:SIMPLE(filecheck=GENERIC): -target cuda -entry eGenericFloat -stage compute

Texture2D<half>        gHalf;
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
//H1: 'SampleGrad' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf() { outBuf[0] = gHalf.SampleGrad(samp, float2(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)); }

//H4: error[E41400]: static assertion failed
//H4: 'SampleGrad' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4() { outBuf[0] = gHalf4.SampleGrad(samp, float2(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)).x; }

//H3D: error[E41400]: static assertion failed
//H3D: 'SampleGrad' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4_3D() { outBuf[0] = gHalf4_3D.SampleGrad(samp, float3(0.5f), float3(0.01f, 0.0f, 0.0f), float3(0.0f, 0.01f, 0.0f)).x; }

// The guard sits above the shape switch, so the layered (array) arm is rejected too.
//H2DA: error[E41400]: static assertion failed
//H2DA: 'SampleGrad' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4_2DArr() { outBuf[0] = gHalf4_2DArr.SampleGrad(samp, float3(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)).x; }

//HC: error[E41400]: static assertion failed
//HC: 'SampleGrad' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalfCombined() { outBuf[0] = gSamplerHalf4.SampleGrad(float2(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)).x; }

//FLOAT: tex2DGrad<float4
[shader("compute")][numthreads(1, 1, 1)]
void eFloat() { outBuf[0] = gFloat4.SampleGrad(samp, float2(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)).x; }

//UINT: tex2DGrad<uint4
[shader("compute")][numthreads(1, 1, 1)]
void eUint() { outBuf[0] = float(gUint4.SampleGrad(samp, float2(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)).x); }

//INT: tex2DGrad<int4
[shader("compute")][numthreads(1, 1, 1)]
void eInt() { outBuf[0] = float(gInt4.SampleGrad(samp, float2(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)).x); }

//FLOAT2DA: tex2DLayeredGrad<float4
[shader("compute")][numthreads(1, 1, 1)]
void eFloat4_2DArr() { outBuf[0] = gFloat4_2DArr.SampleGrad(samp, float3(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)).x; }

//FLOATC: tex2DGrad<float4
[shader("compute")][numthreads(1, 1, 1)]
void eFloatCombined() { outBuf[0] = gSamplerFloat4.SampleGrad(float2(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f)).x; }

// The guard is a `static_assert(!__isHalf<T>())` over a type predicate, which folds to a
// concrete bool per specialization, so it must not fire while the generic body is checked.
T sampleGradR<T : ITexelElement>(Texture2D<T> t, SamplerState s)
{
    return t.SampleGrad(s, float2(0.5f), float2(0.01f, 0.0f), float2(0.0f, 0.01f));
}
//GENERIC: tex2DGrad<float4
[shader("compute")][numthreads(1, 1, 1)]
void eGenericFloat() { outBuf[0] = sampleGradR<float4>(gFloat4, samp).x; }
