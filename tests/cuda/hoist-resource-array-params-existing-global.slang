// Verifies the `_moduleHasUniformParameterGroupGlobal` bail-out in
// hoistCUDAResourceArrayParamsToParameterGroup: when a module already has a module-scope uniform
// parameter group global (here a user `cbuffer`, for which `collectGlobalUniformParameters` already
// synthesizes the single hardcoded `SLANG_globalParams`), the pass must NOT hoist a qualifying
// kernel's resource array — a second parameter-group global would emit as a duplicate
// `extern "C" __constant__ SLANG_globalParams` and be a hard NVRTC duplicate-symbol error.
//
// This lives in its own file because a module-scope `cbuffer` makes `SLANG_globalParams` appear for
// the whole module, which is incompatible with the multi-entry-point MULTI-NOT case in the sibling
// test hoist-resource-array-params.slang.
//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none -stage compute -entry cbMain

cbuffer Globals
{
    float gScale;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cbMain(
    uint tid: SV_DispatchThreadID,
    RWStructuredBuffer<float> tensors[8],
    StructuredBuffer<uint> indices)
{
    tensors[indices[tid]][tid] = gScale;
}
// The hoist bailed: `tensors` stays on the kernel signature (it was NOT moved into a second
// GlobalParams), so the kernel still takes the fixed-size resource array by value.
// CHECK: cbMain(FixedArray<RWStructuredBuffer
