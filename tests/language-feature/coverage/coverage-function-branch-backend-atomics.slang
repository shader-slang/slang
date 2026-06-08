// Verifies that the new function/branch coverage marker ops lower to
// native atomics on additional text backends. SPIR-V and CUDA have
// dedicated tests because their binding paths are separate; this file
// covers HLSL, GLSL, and Metal emission. WGSL is intentionally covered
// by coverage-wgsl-skip.slang because it is an unsupported target for
// coverage instrumentation today.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage -profile cs_5_0
//TEST:SIMPLE(filecheck=HLSL6):-target hlsl -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage -profile cs_6_0
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage -profile glsl_450
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = tid.x;
    if ((tid.x & 1u) != 0u)
        accum += 1u;
    else
        accum += 2u;
    outputBuffer[0] = accum;
}

// cs_5_0 is below shader model 6.0, so it stays on the per-lane path: a plain
// InterlockedAdd(slot, 1, ...) with no wave intrinsics. The -NOT lines pin that
// boundary against the cs_6_0 (HLSL6) aggregated path below.
//HLSL: _slang_coverage
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(0\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(1\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(2\)}}
//HLSL-NOT: InterlockedAdd{{.*_slang_coverage.*int\(3\)}}
//HLSL-NOT: WaveActiveCountBits
//HLSL-NOT: WaveIsFirstLane

// cs_6_0 is shader model 6.0+, so it takes the wave-aggregated path (issue
// #11509): literal WaveActiveCountBits/WaveIsFirstLane intrinsics, and each
// guarded InterlockedAdd adds the active-lane count (captured and bound to the
// slot here) instead of a literal 1.
//HLSL6: _slang_coverage
//HLSL6: [[CNT0:_S[0-9]+]] = WaveActiveCountBits(true)
//HLSL6: WaveIsFirstLane
//HLSL6: InterlockedAdd{{.*}}_slang_coverage{{.*}}int(0){{.*}}[[CNT0]]
//HLSL6: [[CNT1:_S[0-9]+]] = WaveActiveCountBits(true)
//HLSL6: WaveIsFirstLane
//HLSL6: InterlockedAdd{{.*}}_slang_coverage{{.*}}int(1){{.*}}[[CNT1]]
//HLSL6: [[CNT2:_S[0-9]+]] = WaveActiveCountBits(true)
//HLSL6: WaveIsFirstLane
//HLSL6: InterlockedAdd{{.*}}_slang_coverage{{.*}}int(2){{.*}}[[CNT2]]
//HLSL6-NOT: InterlockedAdd{{.*_slang_coverage.*int\(3\)}}

//GLSL: _slang_coverage
//GLSL-DAG: atomicAdd{{.*_slang_coverage.*uint\(0\)}}
//GLSL-DAG: atomicAdd{{.*_slang_coverage.*uint\(1\)}}
//GLSL-DAG: atomicAdd{{.*_slang_coverage.*uint\(2\)}}
//GLSL-NOT: atomicAdd{{.*_slang_coverage.*uint\(3\)}}

// Metal is wave-capable (SIMD-group functions), so coverage increments are
// wave-aggregated (issue #11509): WaveActiveCountBits lowers to
// simd_ballot/popcount (assert those real idioms, not just the kept function
// name), simd_is_first() elects one lane, and the guarded
// atomic_fetch_add_explicit adds the active-lane count. Capture each slot
// pointer and lane count and bind them into the atomic so the test proves the
// atomic targets the slot and adds the lane count.
//METAL: _slang_coverage
//METAL-DAG: simd_ballot
//METAL-DAG: popcount
// slot 0
//METAL: [[MSLOT0:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(0)
//METAL: [[MCNT0:_S[0-9]+]] = WaveActiveCountBits
//METAL: simd_is_first
//METAL: atomic_fetch_add_explicit{{.*}}[[MSLOT0]]{{.*}}, [[MCNT0]],
// slot 1
//METAL: [[MSLOT1:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(1)
//METAL: [[MCNT1:_S[0-9]+]] = WaveActiveCountBits
//METAL: atomic_fetch_add_explicit{{.*}}[[MSLOT1]]{{.*}}, [[MCNT1]],
// slot 2
//METAL: [[MSLOT2:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(2)
//METAL: [[MCNT2:_S[0-9]+]] = WaveActiveCountBits
//METAL: atomic_fetch_add_explicit{{.*}}[[MSLOT2]]{{.*}}, [[MCNT2]],
//METAL-NOT: _slang_coverage{{.*}}int(3)
