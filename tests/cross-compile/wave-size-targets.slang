// wave-size-targets.slang

// `[WaveSize(N)]` on each target:
// - HLSL keeps `[WaveSize(N)]`.
// - WGSL emits `@subgroup_size(N)` with the `subgroups` and `subgroup_size_control` extensions.
// - SPIR-V has no in-shader form (the size is requested at pipeline creation, see
//   VK_EXT_subgroup_size_control); the value is available through reflection.
// - Metal cannot request a SIMD-group width, so the attribute is ignored with a warning.
// - CUDA warps are always 32 wide: `[WaveSize(32)]` needs nothing, any other size is ignored
//   with a warning.

//TEST:SIMPLE(filecheck=HLSL):  -target hlsl  -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=WGSL):  -target wgsl  -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -warnings-disable 56113
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CUDA):  -target cuda  -stage compute -entry computeMain

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[WaveSize(32)]
/*DIAG:
 ^^^^^^^^ 'WaveSize' attribute is not supported on target 'metal' and will be ignored
 ^^^^^^^^ the target has no way to request a wave (SIMD-group) size, so the entry point runs at the size the device chooses; WaveGetLaneCount() returns it
*/
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = WaveActiveSum(dispatchThreadID.x);
}

// HLSL: [WaveSize(32)]

// WGSL-DAG: enable subgroups;
// WGSL-DAG: enable subgroup_size_control;
// WGSL: @compute
// WGSL-NEXT: @workgroup_size(64, 1, 1)
// WGSL-NEXT: @subgroup_size(32)

// METAL-NOT: WaveSize
// METAL: {{\[\[kernel\]\]}}
// METAL-NOT: WaveSize

// CUDA-NOT: WaveSize
// CUDA: void computeMain(
// CUDA-NOT: WaveSize


// WGSL requires the workgroup's x dimension to be a multiple of the subgroup size.
//DIAGNOSTIC_TEST:SIMPLE(diag=XDIM): -target wgsl -stage compute -entry badWorkgroupMain

[shader("compute")]
[WaveSize(32)]
/*XDIM:
 ^^^^^^^^ the workgroup's x dimension (8) must be a multiple of the 'WaveSize' (32) on target 'wgsl'
 ^^^^^^^^ WGSL's '@subgroup_size' requires the x dimension of '@workgroup_size' to be a multiple of the subgroup size; otherwise pipeline creation fails
*/
[numthreads(8, 8, 1)]
void badWorkgroupMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = WaveActiveSum(dispatchThreadID.x);
}


// CUDA cannot run a kernel at any other wave size than 32.
//DIAGNOSTIC_TEST:SIMPLE(diag=CUDA64): -target cuda -stage compute -entry wave64Main

[shader("compute")]
[WaveSize(64)]
/*CUDA64:
 ^^^^^^^^ 'WaveSize(64)' cannot be honored on target 'cuda', whose wave size is always 32; the attribute will be ignored
 ^^^^^^^^ the entry point runs with 32-lane waves, so code that assumes 64 lanes per wave computes wrong results
*/
[numthreads(64, 1, 1)]
void wave64Main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = WaveActiveSum(dispatchThreadID.x);
}


// `[WaveSize]` accepts a link-time constant expression, which is not an integer literal until
// `simplifyIR` has folded it. The check on the entry point must neither abort on it nor look
// at the target's wave size, so SPIR-V (which never reads the value) and WGSL still compile.
//TEST:SIMPLE(filecheck=LINKTIME-SPIRV): -target spirv-asm -stage compute -entry linkTimeWaveSizeMain
//TEST:SIMPLE(filecheck=LINKTIME-WGSL): -target wgsl -stage compute -entry linkTimeWaveSizeMain

extern static const int kLinkTimeWaveSize = 16;

[shader("compute")]
[WaveSize(kLinkTimeWaveSize * 2)]
[numthreads(64, 1, 1)]
void linkTimeWaveSizeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = WaveActiveSum(dispatchThreadID.x);
}

// LINKTIME-SPIRV: OpEntryPoint GLCompute
// LINKTIME-WGSL: @workgroup_size(64, 1, 1)
// LINKTIME-WGSL-NEXT: @subgroup_size(32)
