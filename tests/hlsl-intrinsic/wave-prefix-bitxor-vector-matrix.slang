//TEST_CATEGORY(wave, compute)
// Run WavePrefixBitXor and WaveMaskPrefixBitXor on vector and matrix values and check the results.
// The lane values have overlapping bits, so a prefix OR gives different results from a prefix XOR.
// WavePrefixBitXor reaches the vector and matrix WaveMaskPrefixBitXor overloads only on CUDA
// (D3D12 and Vulkan use their own prefix XOR), so the test also calls WaveMaskPrefixBitXor directly.
//
// Note: DX12/HLSL requires SM 6.5 because the implementation uses WaveMultiPrefixBit* intrinsics

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_5 -shaderobj -render-feature hardware-device
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -capability cuda_sm_7_0 -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

uint packResults(uint2 v, uint2x2 m)
{
    // One hex digit per result, from the lowest digit up: v.x, v.y, m[0][0], m[0][1], m[1][0],
    // m[1][1]. All results are less than 8.
    return v.x | (v.y << 4) | (m[0][0] << 8) | (m[0][1] << 12) | (m[1][0] << 16) | (m[1][1] << 20);
}

[numthreads(8, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint idx = dispatchThreadID.x;
    uint2 v = uint2(idx + 1, 1);
    uint2x2 m = uint2x2(idx + 1, 1, 7, idx);

    outputBuffer[idx] = packResults(WavePrefixBitXor(v), WavePrefixBitXor(m));

    WaveMask mask = WaveGetActiveMask();
    outputBuffer[8 + idx] = packResults(WaveMaskPrefixBitXor(mask, v), WaveMaskPrefixBitXor(mask, m));
}

// Lane i gets the XOR of the values of lanes 0..i-1:
//   idx + 1: 0 1 3 0 4 1 7 0
//   1:       0 1 0 1 0 1 0 1
//   7:       0 7 0 7 0 7 0 7
//   idx:     0 0 1 3 0 4 1 7
// WavePrefixBitXor, then WaveMaskPrefixBitXor with the active mask; both give the same values.
// CHECK: 0
// CHECK-NEXT: 71111
// CHECK-NEXT: 100303
// CHECK-NEXT: 371010
// CHECK-NEXT: 404
// CHECK-NEXT: 471111
// CHECK-NEXT: 100707
// CHECK-NEXT: 771010
// CHECK-NEXT: 0
// CHECK-NEXT: 71111
// CHECK-NEXT: 100303
// CHECK-NEXT: 371010
// CHECK-NEXT: 404
// CHECK-NEXT: 471111
// CHECK-NEXT: 100707
// CHECK-NEXT: 771010
