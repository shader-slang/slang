//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOPRIVATE): -target wgsl -stage compute -entry computeMain
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain

// Runtime-indexed `static const` lookup tables are emitted as module-scope WGSL `const`
// declarations, not as `var<private>`.
//
// WGSL's minimum-supported limit on the combined size of the private variables a shader uses is
// 8192 bytes, and WebKit enforces it. A `const` is not a variable, so tables emitted as `const`
// do not count against that limit. Here two 4 KB tables and a small one would come to 8320 bytes
// of private variables; the tables below are shortened, because the size does not change how
// they are emitted.

static const float kTableA[8] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
static const float kTableB[8] = { 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0 };
static const float4 kVectors[2] = { float4(1.0, 2.0, 3.0, 4.0), float4(5.0, 6.0, 7.0, 8.0) };

// NOPRIVATE holds only a NOT check, so it scans the whole output.
// NOPRIVATE: result code = 0
// NOPRIVATE-NOT: var<private>

// WGSL-DAG: {{^}}const kTableA{{.*}} : array<f32, i32(8)> = array<f32, i32(8)>(
// WGSL-DAG: {{^}}const kTableB{{.*}} : array<f32, i32(8)> = array<f32, i32(8)>(
// WGSL-DAG: {{^}}const kVectors{{.*}} : array<vec4<f32>, i32(2)> = array<vec4<f32>, i32(2)>(

RWStructuredBuffer<float> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    // Run-time indices into each table.
    // WGSL: kTableA{{.*}}[
    // SPV: OpEntryPoint
    float v = kTableA[i % 8] + kTableB[(i * 3) % 8] + kVectors[i & 1][i & 3];
    outputBuffer[i] = v;
}
