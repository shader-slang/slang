//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=FINITE): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MODULE): -target wgsl -entry moduleScopeMain -stage compute

// In WGSL, `bitcast<f32>(u32(...))` of a constant is a const-expression, and a
// const-expression that evaluates to NaN or infinity is a shader-creation error.
// `asfloat` of constant NaN/infinity bits must be emitted so that the bitcast
// happens at run time and the bit pattern is kept.

// CHECK: fn _slang_nonConstU32(x : u32) -> u32
// CHECK-DAG: bitcast<f32>(_slang_nonConstU32(u32(4294967295)))
// CHECK-DAG: bitcast<f32>(_slang_nonConstU32(u32(4286578688)))
// CHECK-DAG: bitcast<vec2<f32>>(vec2<u32>(_slang_nonConstU32(u32(2139095040)), _slang_nonConstU32(u32(1065353216))))

// A finite constant is still emitted as a plain bitcast.
// FINITE-NOT: _slang_nonConstU32(u32(1073741824))
// FINITE: bitcast<f32>((u32(1073741824)))
// FINITE-NOT: _slang_nonConstU32(u32(1073741824))

RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float nan = asfloat(0xFFFFFFFFu);
    float negInf = asfloat(int(0xFF800000));
    float2 infAndOne = asfloat(uint2(0x7F800000u, 0x3F800000u));
    float two = asfloat(0x40000000u);

    outputBuffer[dispatchThreadID.x] = float4(nan, negInf, infAndOne.x + infAndOne.y, two);
}

// Module-scope constants and globals initialized from non-finite bits. Their initializers are
// either folded into the uses or moved into the entry point, so the helper call never ends up in
// a module-scope `const` or `var<private>` initializer (which WGSL requires to be a
// const-expression).

// MODULE-NOT: const {{.*}}_slang_nonConstU32
// MODULE-NOT: var<private> {{.*}}_slang_nonConstU32
// MODULE-DAG: bitcast<f32>(_slang_nonConstU32(u32(4294967295)))
// MODULE-DAG: bitcast<vec2<f32>>(vec2<u32>(_slang_nonConstU32(u32(2139095040)), _slang_nonConstU32(u32(4286578688))))
// MODULE-DAG: bitcast<f32>(_slang_nonConstU32(u32(2143289345)))
// MODULE-NOT: const {{.*}}_slang_nonConstU32
// MODULE-NOT: var<private> {{.*}}_slang_nonConstU32

static const float kModuleNan = asfloat(0xFFFFFFFFu);
static const float2 kModuleInfPair = asfloat(uint2(0x7F800000u, 0xFF800000u));
static const float kModuleTable[2] = { 1.0, asfloat(0x7FC00001u) };
static float gModuleMutable = asfloat(0xFF800000u);

float readModuleConstants(uint i)
{
    gModuleMutable += 1.0;
    return kModuleNan + kModuleInfPair.x + kModuleInfPair.y + kModuleTable[i & 1] + gModuleMutable;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void moduleScopeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = float4(readModuleConstants(dispatchThreadID.x));
}
