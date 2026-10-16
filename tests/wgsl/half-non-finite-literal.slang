//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOBARE): -target wgsl -stage compute -entry computeMain
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain

// Regression test for issue #13672.
//
// WGSL has no literal for an f16 NaN or infinity, and a const-expression that evaluates to one is
// a shader-creation error. A `half` float literal whose value is NaN or an infinity, including one
// that becomes infinite when `IRBuilder::getFloatValue` rounds it to f16, is therefore emitted as
// a call to an f16 prelude helper, as a `float` one already is.

// NOBARE has a separate prefix so its NOT check covers all emitted WGSL after the result-code
// anchor, rather than only the gaps between positive WGSL matches.
// NOBARE: result code = 0
// NOBARE-NOT: {{infh|nanh}}

// The helpers reinterpret bit patterns rather than divide by zero at run time (issue #13658).
// WGSL: enable f16;
// WGSL-DAG: fn _slang_getInfinityHalf(positive: bool) -> f16
// WGSL-DAG: let bits = select(0xfc00u, 0x7c00u, positive);
// WGSL-DAG: return bitcast<vec2<f16>>(bits).x;
// WGSL-DAG: fn _slang_getNanHalf() -> f16
// WGSL-DAG: let bits = 0x7e00u;
// WGSL-DAG: fn _slang_getInfinity(positive: bool) -> f32
// WGSL-DAG: let bits = select(0xff800000u, 0x7f800000u, positive);
// WGSL-DAG: return bitcast<f32>(bits);
// WGSL-DAG: fn _slang_getNan() -> f32
// WGSL-DAG: let bits = 0x7fc00000u;

RWStructuredBuffer<half> outputBuffer;
RWStructuredBuffer<float> floatBuffer;

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
    // 70000 is above the f16 range, so the half constant is an infinity.
    // WGSL: outputBuffer{{.*}} = _slang_getInfinityHalf(true);
    outputBuffer[tid.x + 3] = half(70000.0);
    // WGSL: outputBuffer{{.*}} = 1.5h;
    outputBuffer[tid.x + 4] = half(1.5);

    // The vector is read once, so it folds into its use; a vector read more than once is bound to
    // a WGSL `const`, which cannot hold the helper call (issue #13658).
    // WGSL: outputBuffer{{.*}} = vec3<f16>(_slang_getInfinityHalf(false), 0.0h, 1.0h)[
    half3 v = half3(half(-1.0 / 0.0), half(0.0), half(1.0));
    outputBuffer[tid.x + 5] = v[tid.x % 3];

    // WGSL: floatBuffer{{.*}} = _slang_getInfinity(true);
    floatBuffer[tid.x] = 1.0 / 0.0;
    // WGSL: floatBuffer{{.*}} = _slang_getInfinity(false);
    floatBuffer[tid.x + 1] = -1.0 / 0.0;
    // WGSL: floatBuffer{{.*}} = _slang_getNan();
    floatBuffer[tid.x + 2] = 0.0 / 0.0;
}
