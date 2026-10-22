// wave-size-targets.slang

// `[WaveSize(N)]` on each target:
// - HLSL keeps `[WaveSize(N)]`.
// - WGSL emits `@subgroup_size(N)` with the `subgroups` and `subgroup_size_control` extensions.
// - SPIR-V has no in-shader form (the size is requested at pipeline creation, see
//   VK_EXT_subgroup_size_control); the value is available through reflection.
// - Metal cannot request a SIMD-group width, so the attribute is ignored with a warning.

//TEST:SIMPLE(filecheck=HLSL):  -target hlsl  -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=WGSL):  -target wgsl  -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG): -target metal -stage compute -entry computeMain

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[WaveSize(32)]
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
/*DIAG:
     ^^^^^^^^^^^ 'WaveSize' attribute is not supported on target 'metal' and will be ignored
     ^^^^^^^^^^^ the target has no way to request a wave (SIMD-group) size, so the entry point runs at the size the device chooses; WaveGetLaneCount() returns it
*/
{
    outputBuffer[dispatchThreadID.x] = WaveActiveSum(dispatchThreadID.x);
}

// HLSL: [WaveSize(32)]

// WGSL-DAG: enable subgroups;
// WGSL-DAG: enable subgroup_size_control;
// WGSL: @compute
// WGSL-NEXT: @workgroup_size(64, 1, 1)
// WGSL-NEXT: @subgroup_size(32)

// METAL: {{\[\[kernel\]\]}}
// METAL-NOT: WaveSize


// WGSL requires the workgroup's x dimension to be a multiple of the subgroup size.
//DIAGNOSTIC_TEST:SIMPLE(diag=XDIM): -target wgsl -stage compute -entry badWorkgroupMain

[shader("compute")]
[WaveSize(32)]
[numthreads(8, 8, 1)]
void badWorkgroupMain(uint3 dispatchThreadID: SV_DispatchThreadID)
/*XDIM:
     ^^^^^^^^^^^^^^^^ the workgroup's x dimension (8) must be a multiple of the 'WaveSize' (32) on target 'wgsl'
     ^^^^^^^^^^^^^^^^ WGSL's '@subgroup_size' requires the x dimension of '@workgroup_size' to be a multiple of the subgroup size; otherwise pipeline creation fails
*/
{
    outputBuffer[dispatchThreadID.x] = WaveActiveSum(dispatchThreadID.x);
}
