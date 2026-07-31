// https://github.com/shader-slang/slang/issues/12277
//
// A read-only texture Load on CUDA lowers to the prelude `tex*fetch_int<T>`
// templates, which have no half implementation (float/uint/int only), so a half
// texel must be diagnosed (E41400) rather than emitting broken CUDA.
// The supported controls below and the RWTexture surface path must stay clean.
// Full half-texel fetch support is tracked by #10024.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=H1): -target cuda -entry eHalf -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H2): -target cuda -entry eHalf2 -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H3): -target cuda -entry eHalf3 -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H4): -target cuda -entry eHalf4 -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H3D): -target cuda -entry eHalf4_3D -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H1D): -target cuda -entry eHalf_1D -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H1DA): -target cuda -entry eHalf_1DArr -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=H2DA): -target cuda -entry eHalf4_2DArr -stage compute

//TEST:SIMPLE(filecheck=FLOAT): -target cuda -entry eFloat -stage compute
//TEST:SIMPLE(filecheck=UINT): -target cuda -entry eUint -stage compute
//TEST:SIMPLE(filecheck=INT): -target cuda -entry eInt -stage compute
//TEST:SIMPLE(filecheck=FLOAT2DA): -target cuda -entry eFloat4_2DArr -stage compute
//TEST:SIMPLE(filecheck=RWHALF): -target cuda -entry eRWHalf -stage compute

Texture2D<half>   gHalf;
Texture2D<half2>  gHalf2;
Texture2D<half3>  gHalf3;
Texture2D<half4>  gHalf4;
Texture3D<half4>  gHalf4_3D;
Texture1D<half>        gHalf_1D;
Texture1DArray<half>   gHalf_1DArr;
Texture2DArray<half4>  gHalf4_2DArr;
Texture2D<float4> gFloat4;
Texture2D<uint4>  gUint4;
Texture2D<int4>   gInt4;
Texture2DArray<float4> gFloat4_2DArr;
RWTexture2D<half4> gRWHalf4;

RWStructuredBuffer<float> outBuf;

//H1: error[E41400]: static assertion failed
//H1: 'Load' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf() { outBuf[0] = gHalf.Load(int3(0, 0, 0)); }

//H2: error[E41400]: static assertion failed
//H2: 'Load' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf2() { outBuf[0] = gHalf2.Load(int3(0, 0, 0)).x; }

//H3: error[E41400]: static assertion failed
//H3: 'Load' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf3() { outBuf[0] = gHalf3.Load(int3(0, 0, 0)).x; }

//H4: error[E41400]: static assertion failed
//H4: 'Load' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4() { outBuf[0] = gHalf4.Load(int3(0, 0, 0)).x; }

//H3D: error[E41400]: static assertion failed
//H3D: 'Load' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4_3D() { outBuf[0] = gHalf4_3D.Load(int4(0, 0, 0, 0)).x; }

// The guard sits above the shape switch, so the 1D and array fetch arms are rejected too.
//H1D: error[E41400]: static assertion failed
//H1D: 'Load' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf_1D() { outBuf[0] = gHalf_1D.Load(int2(0, 0)); }

//H1DA: error[E41400]: static assertion failed
//H1DA: 'Load' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf_1DArr() { outBuf[0] = gHalf_1DArr.Load(int3(0, 0, 0)); }

//H2DA: error[E41400]: static assertion failed
//H2DA: 'Load' of a half-typed texture is not supported for the 'cuda' target
[shader("compute")][numthreads(1, 1, 1)]
void eHalf4_2DArr() { outBuf[0] = gHalf4_2DArr.Load(int4(0, 0, 0, 0)).x; }

//FLOAT: tex2Dfetch_int<float4
//FLOAT-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eFloat() { outBuf[0] = gFloat4.Load(int3(0, 0, 0)).x; }

//UINT: tex2Dfetch_int<uint4
//UINT-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eUint() { outBuf[0] = float(gUint4.Load(int3(0, 0, 0)).x); }

//INT: tex2Dfetch_int<int4
//INT-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eInt() { outBuf[0] = float(gInt4.Load(int3(0, 0, 0)).x); }

// Positive control for the array fetch arm: a supported texel type still emits cleanly.
//FLOAT2DA: tex2DArrayfetch_int<float4
//FLOAT2DA-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eFloat4_2DArr() { outBuf[0] = gFloat4_2DArr.Load(int4(0, 0, 0, 0)).x; }

// The RWTexture<half4> surface read IS supported (software conversion) and must not be rejected.
//RWHALF: surf2Dread<__half4
//RWHALF-NOT: static assertion failed
[shader("compute")][numthreads(1, 1, 1)]
void eRWHalf() { outBuf[0] = gRWHalf4.Load(int2(0, 0)).x; }
