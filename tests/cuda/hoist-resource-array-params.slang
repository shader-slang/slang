// Verifies hoistCUDAResourceArrayParamsToParameterGroup: a CUDA compute entry point whose uniform
// parameters transitively contain a fixed-size array of resources is hoisted into a module-scope
// `ConstantBuffer<GlobalParams>` (emitted as `extern "C" __constant__ ... SLANG_globalParams`),
// while entry points without such an array keep the default entry-point parameter path. On CUDA,
// ordinary entry-point uniforms are left on the kernel signature, so `SLANG_globalParams` appears
// here only when the hoist fires.

//TEST:SIMPLE(filecheck=POS): -target cuda -line-directive-mode none -stage compute -entry positiveMain
//TEST:SIMPLE(filecheck=NEGNOARR): -target cuda -line-directive-mode none -stage compute -entry noResourceArrayMain
//TEST:SIMPLE(filecheck=NEGNONRES): -target cuda -line-directive-mode none -stage compute -entry nonResourceArrayMain

// Positive: a runtime-indexed fixed-size resource array uniform must be hoisted into __constant__.
[shader("compute")]
[numthreads(1, 1, 1)]
void positiveMain(
    uint tid: SV_DispatchThreadID,
    RWStructuredBuffer<float> tensors[8],
    StructuredBuffer<uint> indices)
{
    tensors[indices[tid]][tid] = 1.0f;
}
// POS: __constant__{{.*}}SLANG_globalParams

// Negative: a single resource (not a fixed-size array) must not trigger the hoist.
[shader("compute")]
[numthreads(1, 1, 1)]
void noResourceArrayMain(uint tid: SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    output[tid] = 1.0f;
}
// NEGNOARR-NOT: SLANG_globalParams

// Negative: a fixed-size array of non-resource elements must not trigger the hoist.
[shader("compute")]
[numthreads(1, 1, 1)]
void nonResourceArrayMain(
    uint tid: SV_DispatchThreadID,
    RWStructuredBuffer<float> output,
    uniform float scales[8])
{
    output[tid] = scales[tid % 8];
}
// NEGNONRES-NOT: SLANG_globalParams
