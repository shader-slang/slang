// Verifies hoistCUDAResourceArrayParamsToParameterGroup: a CUDA compute entry point whose uniform
// parameters transitively contain a fixed-size array of resources is hoisted into a module-scope
// `ConstantBuffer<GlobalParams>` (emitted as `extern "C" __constant__ ... SLANG_globalParams`),
// while entry points without such an array keep the default entry-point parameter path. On CUDA,
// ordinary entry-point uniforms are left on the kernel signature, so `SLANG_globalParams` appears
// here only when the hoist fires.

//TEST:SIMPLE(filecheck=POS): -target cuda -line-directive-mode none -stage compute -entry positiveMain
//TEST:SIMPLE(filecheck=TRANS): -target cuda -line-directive-mode none -stage compute -entry transitiveMain
//TEST:SIMPLE(filecheck=TENSOR): -target cuda -line-directive-mode none -stage compute -entry tensorWrapperMain
//TEST:SIMPLE(filecheck=MIXED): -target cuda -line-directive-mode none -stage compute -entry mixedOrderMain
//TEST:SIMPLE(filecheck=NEGNOARR): -target cuda -line-directive-mode none -stage compute -entry noResourceArrayMain
//TEST:SIMPLE(filecheck=NEGNONRES): -target cuda -line-directive-mode none -stage compute -entry nonResourceArrayMain
// Compiling every entry point together makes more than one compute kernel qualify, so the driver
// must bail and hoist nothing (a single module cannot hold two `SLANG_globalParams` symbols).
//TEST:SIMPLE(filecheck=MULTI): -target cuda -line-directive-mode none

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
// The hoisted struct holds the uniform params in declaration order (tensors before indices), the
// varying SV_DispatchThreadID is computed locally (not hoisted), and the kernel signature is empty.
// POS: struct GlobalParams
// POS: tensors
// POS: indices
// POS: __constant__{{.*}}SLANG_globalParams
// POS-NOT: positiveMain({{.+}})
// POS: blockIdx

// Positive (transitive): a fixed-size resource array nested inside a struct uniform must also be
// hoisted, exercising the recursive walk through struct fields.
struct Tensors
{
    RWStructuredBuffer<float> bufs[8];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void transitiveMain(uint tid: SV_DispatchThreadID, Tensors t, StructuredBuffer<uint> indices)
{
    t.bufs[indices[tid]][tid] = 1.0f;
}
// TRANS: __constant__{{.*}}SLANG_globalParams

// Positive (struct-wrapped element): a fixed-size array whose element is a struct wrapping a
// resource (the SlangPy tensor shape) must be hoisted, exercising the recursive walk into the
// array element type.
struct Tensor
{
    RWStructuredBuffer<float> data;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void tensorWrapperMain(
    uint tid: SV_DispatchThreadID,
    Tensor tensors[8],
    StructuredBuffer<uint> indices)
{
    tensors[indices[tid]].data[tid] = 1.0f;
}
// TENSOR: __constant__{{.*}}SLANG_globalParams

// Positive (mixed ordering): a uniform resource array declared *before* the varying
// SV_DispatchThreadID must still hoist with correct field indexing — the per-parameter counter has
// to account for the varying parameter's position, not assume the varying one comes first.
[shader("compute")]
[numthreads(1, 1, 1)]
void mixedOrderMain(
    RWStructuredBuffer<float> tensors[8],
    uint tid: SV_DispatchThreadID,
    StructuredBuffer<uint> indices)
{
    tensors[indices[tid]][tid] = 1.0f;
}
// MIXED: struct GlobalParams
// MIXED: tensors
// MIXED: indices
// MIXED: __constant__{{.*}}SLANG_globalParams
// MIXED: blockIdx

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

// Multi-kernel module (all entry points compiled together): more than one compute kernel qualifies,
// so the driver bails and nothing is hoisted.
// MULTI-NOT: SLANG_globalParams
