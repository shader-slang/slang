//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOBARE): -target wgsl -stage compute -entry computeMain
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain

// Regression test for issue #13672.
//
// WGSL has no literal for an f16 NaN or infinity, and a const-expression that evaluates to one is
// a shader-creation error. A `half` NaN/infinity literal, including a finite value that overflows
// f16, is therefore emitted as a call to an f16 prelude helper, as `float` already is.

// NOBARE-NOT: {{infh|nanh}}

// WGSL: enable f16;
// WGSL-DAG: fn _slang_getInfinityHalf(positive: bool) -> f16
// WGSL-DAG: fn _slang_getNanHalf() -> f16

RWStructuredBuffer<half> outputBuffer;

// WGSL: fn computeMain
// SPV: OpEntryPoint
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // WGSL: outputBuffer{{.*}} = _slang_getInfinityHalf(true);
    outputBuffer[tid.x] = half(1.0 / 0.0);
    // WGSL: outputBuffer{{.*}} = _slang_getInfinityHalf(false);
    outputBuffer[tid.x + 1] = half(-1.0 / 0.0);
    // WGSL: outputBuffer{{.*}} = _slang_getNanHalf();
    outputBuffer[tid.x + 2] = half(0.0 / 0.0);
    // A finite value that overflows f16 folds to an infinity.
    // WGSL: outputBuffer{{.*}} = _slang_getInfinityHalf(true);
    outputBuffer[tid.x + 3] = half(70000.0);
    // WGSL: outputBuffer{{.*}} = 1.5h;
    outputBuffer[tid.x + 4] = half(1.5);

    // A function-local vector holding an infinity, read at a runtime index.
    // WGSL: outputBuffer{{.*}} = vec3<f16>(_slang_getInfinityHalf(false), 0.0h, 1.0h)[
    half3 v = half3(half(-1.0 / 0.0), half(0.0), half(1.0));
    outputBuffer[tid.x + 5] = v[tid.x % 3];
}
