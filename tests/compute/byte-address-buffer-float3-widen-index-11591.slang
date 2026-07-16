// byte-address-buffer-float3-widen-index-11591.slang

// Regression for https://github.com/shader-slang/slang/issues/11545 — slice 2 (#11591).
//
// On targets that translate byte-address buffers into structured buffers (GLSL/SPIR-V), the
// equivalent `StructuredBuffer<float3>` runtime array is laid out with the std430 array-element
// stride, which pads a `float3` to 16 bytes (`OpDecorate ... ArrayStride 16`). A base-aligned wide
// `float3` access now takes the whole-value structured-buffer path, so the byte-offset-to-index
// division must divide by that storage stride (16), not the 12-byte natural stride — otherwise a
// runtime location indexes the wrong element (byte offset 48 -> 48/12 = 4 instead of 3).

// Value test: the scratch buffer is *prefilled* with a distinct value at each 16-byte-strided
// float3 slot, and each thread only *loads* (never stores) from its own base-aligned slot, so the
// load's index division is tested against independently-initialized data — not against its own
// possibly-mis-indexed store. Thread 3 reads byte offset 48: the correct `/16` yields element 3
// (value 4.0), while a regressed `/12` would yield element 4 (the distinct sentinel 99.0), so a
// wrong divisor fails the readback rather than passing vacuously. Vulkan exercises the structured-
// buffer path; CPU exercises the direct byte-address path.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Codegen test: the index division uses the std430 array stride (16), never the natural stride 12.
//TEST:SIMPLE(filecheck=CHECKIR):-target spirv -emit-spirv-directly -entry computeMain -stage compute

// Slot i (bytes [i*16, i*16+12)) holds float3(i+1, 0, 0); slot 4 holds the sentinel float3(99,0,0)
// that a `/12`-regressed thread 3 would land on. Only .x is asserted, so the padding lanes are 0.
//TEST_INPUT:ubuffer(data=[1.0 0.0 0.0 0.0 2.0 0.0 0.0 0.0 3.0 0.0 0.0 0.0 4.0 0.0 0.0 0.0 99.0 0.0 0.0 0.0], stride=4):name=scratch
RWByteAddressBuffer scratch;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Load only, from prefilled data, at this thread's base-aligned slot. The wide access must
    // divide byte offset by the 16-byte storage stride to land on element tid.x.
    float3 v = scratch.LoadAligned<float3>(tid.x * 16, 16);
    // CHECK: 1.0
    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 4.0
    outputBuffer[tid.x] = v.x;
}

// The equivalent structured buffer's element array is 16-byte-strided, and the index division uses
// that same 16 (`%uint_16`) — never the natural stride 12. If the divisor regressed to 12 a
// `%uint_12` constant would appear and feed the `OpUDiv`.
// CHECKIR-DAG: OpDecorate %_runtimearr_v3float ArrayStride 16
// CHECKIR-DAG: %uint_16 = OpConstant %uint 16
// CHECKIR: OpUDiv %uint {{.*}} %uint_16
// CHECKIR-NOT: %uint_12 = OpConstant %uint 12
