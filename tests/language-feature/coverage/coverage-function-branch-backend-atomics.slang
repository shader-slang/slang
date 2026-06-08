// Verifies that the new function/branch coverage marker ops lower to
// native atomics on additional text backends. SPIR-V and CUDA have
// dedicated tests because their binding paths are separate; this file
// covers HLSL, GLSL, and Metal emission. WGSL is intentionally covered
// by coverage-wgsl-skip.slang because it is an unsupported target for
// coverage instrumentation today.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage -profile cs_5_0
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

//HLSL: _slang_coverage
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(0\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(1\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(2\)}}
//HLSL-NOT: InterlockedAdd{{.*_slang_coverage.*int\(3\)}}

//GLSL: _slang_coverage
//GLSL-DAG: atomicAdd{{.*_slang_coverage.*uint\(0\)}}
//GLSL-DAG: atomicAdd{{.*_slang_coverage.*uint\(1\)}}
//GLSL-DAG: atomicAdd{{.*_slang_coverage.*uint\(2\)}}
//GLSL-NOT: atomicAdd{{.*_slang_coverage.*uint\(3\)}}

// Metal is wave-capable (SIMD-group functions), so coverage increments are
// wave-aggregated (issue #11509): count active lanes, then simd_is_first()
// guards a single atomic per marker.
//METAL: _slang_coverage
//METAL-DAG: WaveActiveCountBits
//METAL-DAG: simd_is_first
//METAL-DAG: _slang_coverage{{.*}}int(0)
//METAL-DAG: _slang_coverage{{.*}}int(1)
//METAL-DAG: _slang_coverage{{.*}}int(2)
//METAL-DAG: atomic_fetch_add_explicit
//METAL-DAG: atomic_fetch_add_explicit
//METAL-DAG: atomic_fetch_add_explicit
//METAL-NOT: _slang_coverage{{.*}}int(3)
