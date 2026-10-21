//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOMODULE): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOCONST): -target wgsl -stage compute -entry computeMain
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain

// Regression test for issue #13658.
//
// Slang's WGSL emitter spells a NaN or infinity literal as a call to the `_slang_getNan` /
// `_slang_getInfinity` prelude helpers, and a module-scope initializer may not call a
// user-declared function. A runtime-indexed `static const` array holding such a literal therefore
// cannot be the module-scope `var<private>` that an all-finite table becomes; it is materialized
// inside the functions that use it instead. For the same reason, a function-scope value built from
// such a literal is declared with `var`, never `const`.

// NOMODULE and NOCONST compile the same shader as WGSL but only hold NOT checks, so each scans the
// whole output rather than the gap before a positive WGSL match. NOMODULE relies on module-scope
// declarations starting at column 0 while function-scope ones are indented.
// NOMODULE: result code = 0
// NOMODULE-NOT: {{^(var|const|override)[ <].*_slang_get}}

// NOCONST: result code = 0
// NOCONST-NOT: const {{.*}}_slang_get

// The helpers build NaN and infinity from bit patterns, not from a runtime `0.0 / 0.0`: the NVIDIA
// Vulkan driver crashes creating a pipeline for a sunk table that holds such a division. The NaN
// below is `0.0 / 0.0` folded on the build host, whose sign differs between x86 and aarch64.
// WGSL: fn _slang_getNan(bits : u32) -> f32
// WGSL-NEXT: {
// WGSL-NEXT: return bitcast<f32>(bits);
// WGSL: fn _slang_getInfinity(positive: bool) -> f32
// WGSL-NEXT: {
// WGSL-NEXT: let bits = select(0xff800000u, 0x7f800000u, positive);
// WGSL-NEXT: return bitcast<f32>(bits);

static const float kTable[3] = { 0.0 / 0.0, 1.0 / 0.0, 2.0 };
static const float kNested[2][2] = { { -1.0 / 0.0, 1.0 }, { 2.0, 3.0 } };
static const float2x2 kMats[2] = { float2x2(1.0 / 0.0, 0.0, 0.0, 1.0), float2x2(1.0, 0.0, 0.0, 1.0) };
static const float4 kVecs[2] = { float4(-1.0 / 0.0, 0.0, 0.0, 0.0), float4(1.0, 2.0, 3.0, 4.0) };
static const float4 kSplats[2] = { float4(1.0 / 0.0), float4(0.0) };
static const float2x2 kMatSplats[2] = { float2x2(0.0 / 0.0), float2x2(1.0) };
static const float kFinite[3] = { 1.0, 2.0, 3.0 };

// WGSL: var<private> kFinite{{.*}} = array<f32, i32(3)>( 1.0f, 2.0f, 3.0f );

RWStructuredBuffer<float> outputBuffer;

// Two functions that share `kTable` each get their own function-scope copy.
// WGSL: fn readTable
// WGSL: var kTable{{.*}} = array<f32, i32(3)>( _slang_getNan(0x{{[7F]}}FC00000u), _slang_getInfinity(true), 2.0f );
[noinline]
float readTable(uint i)
{
    return kTable[i];
}

// WGSL: fn readTableReversed
// WGSL: var kTable{{.*}} = array<f32, i32(3)>( _slang_getNan(0x{{[7F]}}FC00000u), _slang_getInfinity(true), 2.0f );
[noinline]
float readTableReversed(uint i)
{
    return kTable[2 - i];
}

// A local vector that is read more than once is bound to a name; it holds an infinity, so it is a
// `var`.
// WGSL: fn readLocalVector
// WGSL: var {{.*}} : vec4<f32> = vec4<f32>(_slang_getInfinity(false), 0.0f, 0.0f, 0.0f);
[noinline]
float readLocalVector(uint i)
{
    float4 v = float4(-1.0 / 0.0, 0.0, 0.0, 0.0);
    return v[i] + v[(i + 1) & 3];
}

// WGSL: fn computeMain
// WGSL-DAG: var kNested{{.*}} = array<array<f32
// WGSL-DAG: var kMats{{.*}} = array<mat2x2<f32>
// WGSL-DAG: var kVecs{{.*}} = array<vec4<f32>
// WGSL-DAG: var kSplats{{.*}} = array<vec4<f32>
// WGSL-DAG: var kMatSplats{{.*}} = array<mat2x2<f32>
// SPV: OpEntryPoint
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x & 1;
    outputBuffer[tid.x] = readTable(tid.x) + readTableReversed(tid.x) + readLocalVector(tid.x)
        + kNested[i][i] + kMats[i][i][i] + kVecs[i][i] + kSplats[i][i] + kMatSplats[i][i][i]
        + kFinite[tid.x];
}
