//DIAGNOSTIC_TEST:SIMPLE: -target glsl -entry computeMain -stage compute

// Regression test: specialization constants as array sizes crash on GLSL
// (broader issue also affects WGSL/Metal; this test intentionally exercises GLSL)
// Issue: https://github.com/shader-slang/slang/issues/10214

[SpecializationConstant]
const int SPEC_SIZE = 16;

[SpecializationConstant]
const float SPEC_SCALE = 1.0;

struct Data { float values[SPEC_SIZE]; }

RWStructuredBuffer<Data> buffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID) {
    Data d;
    for (int i = 0; i < SPEC_SIZE; i++)
        d.values[i] = float(i) * SPEC_SCALE;
    buffer[0] = d;
}
