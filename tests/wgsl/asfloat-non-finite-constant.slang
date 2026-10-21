//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOCONSTEXPR): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOCONSTEXPR): -target wgsl -stage compute -entry computeMain -O0
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// Regression test for issue #13635.
//
// WGSL's `bitcast` of a literal is a const-expression, and a const-expression that evaluates to NaN
// or infinity is a shader-creation error. `asfloat` of constant NaN/infinity bits must therefore
// produce the value at run time while keeping the exact bit pattern, including a NaN payload.

// NOCONSTEXPR: result code = 0
// NOCONSTEXPR-NOT: bitcast<{{(vec[0-9]<)?}}f32>{{>?}}({{\(?}}{{(vec[0-9]<)?}}{{[iu]}}32

// SPV: OpEntryPoint

// WGSL: fn _slang_getNan(bits : u32) -> f32
// WGSL: fn computeMain
// WGSL-DAG: _slang_getNan(0xFFFFFFFFu)
// WGSL-DAG: _slang_getNan(0x7FC00001u)
// WGSL-DAG: _slang_getInfinity(true)
// WGSL-DAG: _slang_getInfinity(false)

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const float kTable[4] = {
    asfloat(0xFFFFFFFFu), asfloat(0x7F800000u), asfloat(int(0xFF800000u)), asfloat(0x7FC00001u)
};

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputBuffer[0] = asuint(asfloat(0xFFFFFFFFu));
    outputBuffer[1] = asuint(asfloat(0x7F800000u));
    outputBuffer[2] = asuint(asfloat(int(0xFF800000u)));
    outputBuffer[3] = asuint(asfloat(0x7FC00001u));
    float2 v = asfloat(uint2(0xFFFFFFFFu, 0x7F800000u));
    outputBuffer[4] = asuint(v[i]);
    outputBuffer[5] = asuint(v[i + 1]);
    for (uint k = 0; k < 4; k++)
        outputBuffer[6 + k] = asuint(kTable[(i + k) & 3]);
    outputBuffer[10] = asuint(bit_cast<float>(0x7FC00001u));
    outputBuffer[11] = asuint(asfloat(0x3F800000u));
}

// BUF: {{^}}4294967295{{$}}
// BUF-NEXT: {{^}}2139095040{{$}}
// BUF-NEXT: {{^}}4286578688{{$}}
// BUF-NEXT: {{^}}2143289345{{$}}
// BUF-NEXT: {{^}}4294967295{{$}}
// BUF-NEXT: {{^}}2139095040{{$}}
// BUF-NEXT: {{^}}4294967295{{$}}
// BUF-NEXT: {{^}}2139095040{{$}}
// BUF-NEXT: {{^}}4286578688{{$}}
// BUF-NEXT: {{^}}2143289345{{$}}
// BUF-NEXT: {{^}}2143289345{{$}}
// BUF-NEXT: {{^}}1065353216{{$}}
