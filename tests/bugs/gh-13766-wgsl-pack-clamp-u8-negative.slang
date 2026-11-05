// Regression test for #13766: `packUint4x8Clamp(int32_t4)` (`pack_clamp_u8`) must clamp a
// negative lane to 0. On WGSL it was emitted as `pack4xU8Clamp(vec4<u32>(x))`, which converts
// to `u32` before clamping, so a negative lane wrapped and was clamped to 255.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -profile cs_6_6 -render-feature hardware-device -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -g0 -output-using-type

// The WGSL source must clamp the low end in `i32` before the conversion to `u32`.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
// WGSL-COUNT-4: pack4xU8Clamp(vec4<u32>(max({{.*}}, vec4<i32>(0))))

// Inputs are (-1, 300, 5, 0) and (INT_MIN, 255, 256, -129), which pack to 0x0005FF00 (392960)
// and 0x00FFFF00 (16776960).
// CHECK: type: uint32_t
// CHECK-NEXT: 392960
// CHECK-NEXT: 16776960
// CHECK-NEXT: 392960
// CHECK-NEXT: 16776960
//TEST_INPUT:ubuffer(data=[0xFFFFFFFF 300 5 0 0x80000000 255 256 0xFFFFFF7F], stride=4):name=input
StructuredBuffer<int32_t4> input;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = pack_clamp_u8(input[0]);
    outputBuffer[1] = pack_clamp_u8(input[1]);
    outputBuffer[2] = packUint4x8Clamp(input[0]);
    outputBuffer[3] = packUint4x8Clamp(input[1]);
}
