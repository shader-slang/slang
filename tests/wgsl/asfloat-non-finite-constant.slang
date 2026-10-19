//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=FINITE): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MODULE): -target wgsl -entry moduleScopeMain -stage compute
//TEST:SIMPLE(filecheck=MODNOT): -target wgsl -entry moduleScopeMain -stage compute
//TEST:SIMPLE(filecheck=VECTOR): -target wgsl -entry vectorMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry moduleScopeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry vectorMain -stage compute

// In WGSL, `bitcast<f32>(u32(...))` of a constant is a const-expression, and a
// const-expression that evaluates to NaN or infinity is a shader-creation error.
// `asfloat` of constant NaN/infinity bits must be emitted so that the bitcast
// happens at run time and the bit pattern is kept.

// CHECK: fn _slang_nonConstU32(x : u32) -> u32
// CHECK-DAG: bitcast<f32>(_slang_nonConstU32(u32(4294967295)))
// CHECK-DAG: bitcast<f32>(_slang_nonConstU32(u32(4286578688)))
// CHECK-DAG: bitcast<vec2<f32>>(vec2<u32>(_slang_nonConstU32(u32(2139095040)), _slang_nonConstU32(u32(1065353216))))
// CHECK-DAG: bitcast<vec3<f32>>(vec3<u32>(_slang_nonConstU32(u32(2139095040)), _slang_nonConstU32(u32(2139095040)), _slang_nonConstU32(u32(2139095040))))

// The bundled Tint accepts each entry point.
// TINT: OpEntryPoint

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
    // A vector made from one scalar (`MakeVectorFromScalar`).
    float3 infs = asfloat(uint3(0x7F800000u));

    outputBuffer[dispatchThreadID.x] = float4(nan, negInf, infAndOne.x + infAndOne.y, two + infs.y);
}

// Module-scope constants and globals initialized from non-finite bits. Their initializers are
// either folded into the uses or moved into the entry point, so the helper call never ends up in
// a module-scope `const` or `var<private>` initializer (which WGSL requires to be a
// const-expression).

// MODULE-DAG: bitcast<f32>(_slang_nonConstU32(u32(4294967295)))
// MODULE-DAG: bitcast<vec2<f32>>(vec2<u32>(_slang_nonConstU32(u32(2139095040)), _slang_nonConstU32(u32(4286578688))))
// MODULE-DAG: bitcast<f32>(_slang_nonConstU32(u32(2143289345)))

// MODNOT searches the whole output (a NOT next to a DAG group only covers the text between
// matches, which starts mid-line).
// MODNOT-NOT: {{(const|var<private>).*}}_slang_{{nonConstU32|getNan|getInfinity}}(
// MODNOT: fn moduleScopeMain(
// MODNOT-NOT: {{(const|var<private>).*}}_slang_{{nonConstU32|getNan|getInfinity}}(

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

// `bit_cast<float2>` of constant NaN/infinity bits is folded into a vector of float constants
// that WGSL emits as calls (`_slang_getInfinity`, `_slang_getNan`). Used twice, the vector gets a
// local, which must be a `var`: a `const` initializer must be a const-expression.
// VECTOR-NOT: const {{.*}}_slang_
// VECTOR: var {{.*}} : vec2<f32> = vec2<f32>(_slang_getInfinity(true), _slang_getNan());
// VECTOR-NOT: const {{.*}}_slang_
[shader("compute")]
[numthreads(1, 1, 1)]
void vectorMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const float2 v = bit_cast<float2>(uint2(0x7F800000u, 0x7FC00000u));
    outputBuffer[dispatchThreadID.x] = float4(v, v);
}
