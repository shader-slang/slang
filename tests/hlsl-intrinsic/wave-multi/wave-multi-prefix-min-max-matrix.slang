//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -capability cuda_sm_7_0 -compute -shaderobj

// The matrix overloads of WaveMultiPrefix{Inclusive,Exclusive}{Min,Max} and WaveMaskPrefixMin/Max
// apply the vector operation to each row. The -cuda row checks that the matrix overloads are
// available on CUDA, like their scalar and vector forms.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// The CUDA target has no matrix `==`, so compare row by row.
bool areEqual(int2x3 a, int2x3 b)
{
    return all(a[0] == b[0]) & all(a[1] == b[1]);
}

[numthreads(32, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    let lane = dispatchThreadID.x;
    let mask = uint4(0xFFFFFFFF, 0, 0, 0);

    // Lane i holds i + offset in each element, so the prefix minimum over lanes 0..i is the
    // offset and the prefix maximum is the value of the last lane in the range.
    let offset = int2x3(0, 1, 2, 3, 4, 5);
    let value = offset + int(lane);
    let previous = offset + (int(lane) - 1);

    // Every lane in `mask` must execute each operation, so they all run before any lane-dependent
    // test.
    let inclusiveMin = WaveMultiPrefixInclusiveMin(value, mask);
    let inclusiveMax = WaveMultiPrefixInclusiveMax(value, mask);
    let exclusiveMin = WaveMultiPrefixExclusiveMin(value, mask);
    let exclusiveMax = WaveMultiPrefixExclusiveMax(value, mask);
    let maskMin = WaveMaskPrefixMin(mask.x, value);
    let maskMax = WaveMaskPrefixMax(mask.x, value);

    bool ok = areEqual(inclusiveMin, offset) & areEqual(inclusiveMax, value);

    // Lane 0 has no lower lanes, so its exclusive results are identity values that depend on `T`.
    if (lane > 0)
    {
        ok = ok & areEqual(exclusiveMin, offset) & areEqual(exclusiveMax, previous) &
             areEqual(maskMin, offset) & areEqual(maskMax, previous);
    }

    outputBuffer[lane] = ok ? 1 : 0;
}

// CHECK-COUNT-32: 1
