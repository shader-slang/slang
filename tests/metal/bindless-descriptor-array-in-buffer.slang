// Regression for #11970: on Metal, a `StructuredBuffer<DescriptorHandle<R>>`
// (equivalently `StructuredBuffer<R.Handle>`) used to emit an illegal buffer
// parameter. `StructuredBuffer<E>` emits `emitType(E) + " device*"` and a
// bindless `DescriptorHandle<R>` emits `R` directly, so the element collapsed
// to `uint device*` / `texture2d<...>` and the outer buffer appended another
// `device*` — `uint device* device*` (buffer) or `texture2d<...> device*`
// (texture), both of which Apple's compiler rejects for `[[buffer]]`.
//
// The fix wraps the descriptor element in a 1-member struct so the buffer
// becomes a legal `const device Wrapper*` (spvDescriptor / argument-buffer
// tier-2 layout, ABI byte-identical).

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute

// Buffer sub-case: `StructuredBuffer<uint>` handles.
StructuredBuffer<StructuredBuffer<uint>.Handle> indexBuffers;

// Texture sub-case: `Texture2D<float4>` handles.
StructuredBuffer<Texture2D<float4>.Handle> textures;
SamplerState samp;

RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_GroupIndex)
{
    StructuredBuffer<uint> b = indexBuffers[tid];
    Texture2D<float4> t = textures[tid];
    outputBuffer[tid] =
        t.SampleLevel(samp, float2(0, 0), 0) + (float)b[0];
}

// The descriptor elements are wrapped in single-member storage structs, so the
// buffers become one legal level of `device` indirection. There must be no
// double-pointer or bare-texture buffer parameter left.
// CHECK-DAG: struct _DescriptorHandleStorage_
// CHECK: computeMain
// CHECK-NOT: device* device*
// CHECK-NOT: texture2d<{{.*}}> device*

// Apple's compiler must accept the generated MSL end-to-end.
// METALLIB: @computeMain
