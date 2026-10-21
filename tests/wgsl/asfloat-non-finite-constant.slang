//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain -minimum-slang-optimization
//TEST:SIMPLE(filecheck=NOCONSTEXPR): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOCONSTEXPR): -target wgsl -stage compute -entry computeMain -minimum-slang-optimization
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain -minimum-slang-optimization
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type -xslang -minimum-slang-optimization
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// WGSL's `bitcast` of a literal is a const-expression, and a const-expression that evaluates to NaN
// or infinity is a shader-creation error. `asfloat` of constant NaN/infinity bits must therefore
// produce the value at run time. A quiet NaN keeps its exact bits, including the sign and payload.
// The default and `-minimum-slang-optimization` pipelines differ in which passes fold the constant
// bit casts, so each is checked; `-O<n>` does not change the WGSL.

// NOCONSTEXPR compiles the same shader as WGSL but only holds a NOT check, so it scans the whole
// output rather than the gaps between positive WGSL matches. It rejects a `bitcast` of an integer
// literal such as `bitcast<f32>(u32(4294967295))` or `bitcast<vec2<f32>>(vec2<u32>(...))`; the
// helper body's `bitcast<f32>(bits)` takes a parameter, so it does not match.
// NOCONSTEXPR: result code = 0
// NOCONSTEXPR-NOT: bitcast<{{(vec[0-9]<)?}}f32>{{>?}}({{\(?}}{{(vec[0-9]<)?}}{{[iu]}}32

// SPV: OpEntryPoint

// WGSL: fn _slang_getNan(bits : u32) -> f32
// WGSL: fn computeMain
// WGSL-DAG: _slang_getNan(0xFFFFFFFFu)
// WGSL-DAG: _slang_getNan(0x7FC00001u)
// WGSL-DAG: _slang_getInfinity(true)
// WGSL-DAG: _slang_getInfinity(false)

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const float kTable[4] = {
    asfloat(0xFFFFFFFFu), asfloat(0x7F800000u), asfloat(int(0xFF800000u)), asfloat(0x7FC00001u)
};

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // `i` is a run-time zero, so the vector and table reads below are indexed at run time and the
    // constants are materialized rather than folded into each read.
    uint i = tid.x;

    // Slots 0-3: scalar `asfloat` of the uint and int overloads.
    outputBuffer[0] = asuint(asfloat(0xFFFFFFFFu));
    outputBuffer[1] = asuint(asfloat(0x7F800000u));
    outputBuffer[2] = asuint(asfloat(int(0xFF800000u)));
    outputBuffer[3] = asuint(asfloat(0x7FC00001u));

    // Slots 4-5: the vector uint overload. Slots 12-13: the vector int overload.
    float2 v = asfloat(uint2(0xFFFFFFFFu, 0x7F800000u));
    outputBuffer[4] = asuint(v[i]);
    outputBuffer[5] = asuint(v[i + 1]);

    // Slots 6-9: a runtime-indexed `static const` table.
    for (uint k = 0; k < 4; k++)
        outputBuffer[6 + k] = asuint(kTable[(i + k) & 3]);

    // Slot 10: `bit_cast` directly, which isolates the emitter change from the `asfloat` lowering.
    outputBuffer[10] = asuint(bit_cast<float>(0x7FC00001u));
    // Slot 11: a finite control.
    outputBuffer[11] = asuint(asfloat(0x3F800000u));

    float2 w = asfloat(int2(int(0xFF800000u), int(0x7FC00001u)));
    outputBuffer[12] = asuint(w[i]);
    outputBuffer[13] = asuint(w[i + 1]);

    // Slots 14-16: a vector mixing a run-time lane with constant non-finite lanes.
    float3 m = asfloat(uint3(0xFFFFFFFFu, tid.x, 0xFF800000u));
    outputBuffer[14] = asuint(m[i]);
    outputBuffer[15] = asuint(m[i + 1]);
    outputBuffer[16] = asuint(m[i + 2]);
}

// BUF: {{^}}4294967295{{$}}
// 0x7F800000
// BUF-NEXT: {{^}}2139095040{{$}}
// 0xFF800000
// BUF-NEXT: {{^}}4286578688{{$}}
// 0x7FC00001
// BUF-NEXT: {{^}}2143289345{{$}}
// BUF-NEXT: {{^}}4294967295{{$}}
// BUF-NEXT: {{^}}2139095040{{$}}
// BUF-NEXT: {{^}}4294967295{{$}}
// BUF-NEXT: {{^}}2139095040{{$}}
// BUF-NEXT: {{^}}4286578688{{$}}
// BUF-NEXT: {{^}}2143289345{{$}}
// BUF-NEXT: {{^}}2143289345{{$}}
// 0x3F800000
// BUF-NEXT: {{^}}1065353216{{$}}
// BUF-NEXT: {{^}}4286578688{{$}}
// BUF-NEXT: {{^}}2143289345{{$}}
// BUF-NEXT: {{^}}4294967295{{$}}
// BUF-NEXT: {{^}}0{{$}}
// BUF-NEXT: {{^}}4286578688{{$}}
