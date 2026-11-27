//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -xslang -DMATRIX_WAVE_MASK_MATCH
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj

// `WaveMatch` returns, in each lane, the mask of the active lanes that hold the same
// value as that lane. On CUDA, `WaveMaskMatch` does the same over the lanes in its mask
// argument; the other targets ignore that argument, so every call here passes
// `WaveGetActiveMask()`. The lanes here hold different values, so every lane gets the
// mask of its own group rather than the full or an empty mask. Only lanes 0 to 7 run,
// so `.yzw` of every `WaveMatch` result must be 0, which slots 48 to 55 check.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;

    uint4 scalarMask = WaveMatch(idx % 2);

    // Lanes match only when every component matches, so the two components
    // split the lanes into four groups.
    uint4 vectorMask = WaveMatch(uint2(idx % 2, idx / 4));

    uint4 floatMask = WaveMatch(float(idx / 3));

    // Row 0 holds the first key of the vector case and row 1 the second, so the
    // lanes form the same four groups.
    uint4 matrixMask = WaveMatch(float2x2(float(idx % 2), 0.0, 0.0, float(idx / 4)));

    outputBuffer[idx] = scalarMask.x;
    outputBuffer[8 + idx] = vectorMask.x;
    outputBuffer[16 + idx] = floatMask.x;
    outputBuffer[24 + idx] = matrixMask.x;
    outputBuffer[32 + idx] = WaveMaskMatch(WaveGetActiveMask(), idx % 4);

    // Only lanes 0 to 4 are active, so the groups contain only those lanes.
    if (idx < 5)
        outputBuffer[40 + idx] = WaveMatch(idx % 2).x;

    outputBuffer[48 + idx] = scalarMask.y | scalarMask.z | scalarMask.w | vectorMask.y |
                             vectorMask.z | vectorMask.w | floatMask.y | floatMask.z |
                             floatMask.w | matrixMask.y | matrixMask.z | matrixMask.w;

    float2x2 maskKeys = float2x2(float(idx % 3), 0.0, 0.0, float(idx / 4));
#if defined(MATRIX_WAVE_MASK_MATCH)
    outputBuffer[56 + idx] = WaveMaskMatch(WaveGetActiveMask(), maskKeys);
#else
    // The matrix overload of `WaveMaskMatch` has no SPIR-V implementation, so the
    // Vulkan control computes the same mask with `WaveMatch`.
    outputBuffer[56 + idx] = WaveMatch(maskKeys).x;
#endif
}

// 0-7: WaveMatch(idx % 2) -> {0,2,4,6} {1,3,5,7}
// CHECK: {{^}}55{{$}}
// CHECK-NEXT: {{^}}AA{{$}}
// CHECK-NEXT: {{^}}55{{$}}
// CHECK-NEXT: {{^}}AA{{$}}
// CHECK-NEXT: {{^}}55{{$}}
// CHECK-NEXT: {{^}}AA{{$}}
// CHECK-NEXT: {{^}}55{{$}}
// CHECK-NEXT: {{^}}AA{{$}}
// 8-15: WaveMatch(uint2(idx % 2, idx / 4)) -> {0,2} {1,3} {4,6} {5,7}
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}A{{$}}
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}A{{$}}
// CHECK-NEXT: {{^}}50{{$}}
// CHECK-NEXT: {{^}}A0{{$}}
// CHECK-NEXT: {{^}}50{{$}}
// CHECK-NEXT: {{^}}A0{{$}}
// 16-23: WaveMatch(float(idx / 3)) -> {0,1,2} {3,4,5} {6,7}
// CHECK-NEXT: {{^}}7{{$}}
// CHECK-NEXT: {{^}}7{{$}}
// CHECK-NEXT: {{^}}7{{$}}
// CHECK-NEXT: {{^}}38{{$}}
// CHECK-NEXT: {{^}}38{{$}}
// CHECK-NEXT: {{^}}38{{$}}
// CHECK-NEXT: {{^}}C0{{$}}
// CHECK-NEXT: {{^}}C0{{$}}
// 24-31: WaveMatch(float2x2) -> the groups of slots 8-15
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}A{{$}}
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}A{{$}}
// CHECK-NEXT: {{^}}50{{$}}
// CHECK-NEXT: {{^}}A0{{$}}
// CHECK-NEXT: {{^}}50{{$}}
// CHECK-NEXT: {{^}}A0{{$}}
// 32-39: WaveMaskMatch(active, idx % 4) -> {0,4} {1,5} {2,6} {3,7}
// CHECK-NEXT: {{^}}11{{$}}
// CHECK-NEXT: {{^}}22{{$}}
// CHECK-NEXT: {{^}}44{{$}}
// CHECK-NEXT: {{^}}88{{$}}
// CHECK-NEXT: {{^}}11{{$}}
// CHECK-NEXT: {{^}}22{{$}}
// CHECK-NEXT: {{^}}44{{$}}
// CHECK-NEXT: {{^}}88{{$}}
// 40-47: WaveMatch(idx % 2) in lanes 0-4 -> {0,2,4} {1,3}; lanes 5-7 write nothing
// CHECK-NEXT: {{^}}15{{$}}
// CHECK-NEXT: {{^}}A{{$}}
// CHECK-NEXT: {{^}}15{{$}}
// CHECK-NEXT: {{^}}A{{$}}
// CHECK-NEXT: {{^}}15{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// 48-55: OR of every .yzw -> 0
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// 56-63: keys (idx % 3, idx / 4) -> {0,3} {1} {2} {4,7} {5} {6}
// CHECK-NEXT: {{^}}9{{$}}
// CHECK-NEXT: {{^}}2{{$}}
// CHECK-NEXT: {{^}}4{{$}}
// CHECK-NEXT: {{^}}9{{$}}
// CHECK-NEXT: {{^}}90{{$}}
// CHECK-NEXT: {{^}}20{{$}}
// CHECK-NEXT: {{^}}40{{$}}
// CHECK-NEXT: {{^}}90{{$}}
