//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DINPUT_SIZE=64 -xslang -DOUTPUT_SIZE=64 -xslang -DSUBGROUP_COUNT=2 -xslang -DEXPECTED_BYTES=16384
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DINPUT_SIZE=32 -xslang -DOUTPUT_SIZE=32 -xslang -DSUBGROUP_COUNT=2 -xslang -DEXPECTED_BYTES=8192
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DINPUT_SIZE=64 -xslang -DOUTPUT_SIZE=64 -xslang -DSUBGROUP_COUNT=1 -xslang -DEXPECTED_BYTES=8192
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DINPUT_SIZE=128 -xslang -DOUTPUT_SIZE=128 -xslang -DSUBGROUP_COUNT=2 -xslang -DEXPECTED_BYTES=32768
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DINPUT_SIZE=48 -xslang -DOUTPUT_SIZE=64 -xslang -DSUBGROUP_COUNT=2 -xslang -DEXPECTED_BYTES=14336
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DINPUT_SIZE=64 -xslang -DOUTPUT_SIZE=16 -xslang -DSUBGROUP_COUNT=2 -xslang -DEXPECTED_BYTES=10240
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DINPUT_SIZE=32 -xslang -DOUTPUT_SIZE=32 -xslang -DSUBGROUP_COUNT=3 -xslang -DEXPECTED_BYTES=12288
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DINPUT_SIZE=17 -xslang -DOUTPUT_SIZE=33 -xslang -DSUBGROUP_COUNT=1 -xslang -DEXPECTED_BYTES=5120
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER): -vk -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DINPUT_SIZE=32 -xslang -DOUTPUT_SIZE=32 -xslang -DSUBGROUP_COUNT=2 -xslang -DEXPECTED_BYTES=8192

// Exercise the actual calculated scratch allocation, including narrow, wide,
// asymmetric, padded, single-subgroup and odd-subgroup-count layers.
// Run the CUDA cases under Compute Sanitizer to check shared-memory bounds.
import slang.neural;

#if TEST_HALF
typealias DType = half;
#else
typealias DType = float;
#endif

// Both targets use the same linear-layout requirement.
typealias Scratch = SharedMemorySize<DType, TargetEnum.CUDA,
    ExecutionMode.Training, 32, SUBGROUP_COUNT, INPUT_SIZE, OUTPUT_SIZE>;

//TEST_INPUT: set gradients = ubuffer(stride=4, count=16384)
uniform RWStructuredBuffer<DType>.Handle gradients;
//TEST_INPUT: ubuffer(stride=4, count=1):out, name=result
RWStructuredBuffer<uint> result;

[shader("compute")]
[numthreads(32 * SUBGROUP_COUNT, 1, 1)]
void computeMain(uint lane : SV_GroupIndex)
{
    static_assert(Scratch.Bytes == EXPECTED_BYTES, "Unexpected training scratch size.");
    static_assert(SharedMemorySize<half, TargetEnum.CUDA,
        ExecutionMode.Inference, 32, 2, 64, 64>.Bytes == 4096, "Inference scratch changed.");
    static const int ThreadCount = 32 * SUBGROUP_COUNT;
    for (uint i = lane; i < INPUT_SIZE * OUTPUT_SIZE; i += ThreadCount)
        gradients[i] = DType(0);
    AllMemoryBarrierWithGroupSync();

    // Distinct operands and lane/feature-dependent values detect overlapping
    // staging regions and missing subgroup contributions, unlike all-ones data.
    DType input[INPUT_SIZE];
    DType dOut[OUTPUT_SIZE];
    for (uint j = 0; j < INPUT_SIZE; j++)
        input[j] = DType(float((j % 7 + 1) * (lane + 1)) / 128.0);
    for (uint i = 0; i < OUTPUT_SIZE; i++)
        dOut[i] = DType(float(i % 5 + 1) / 16.0);

    neural_unittest.testOuterProductAccumulate<
        DType, INPUT_SIZE, OUTPUT_SIZE, Scratch, SUBGROUP_COUNT>(input, dOut, gradients);

    // Check every matrix entry. Each lane contributes (lane + 1) times the
    // feature-dependent factor, giving a closed-form arithmetic-series sum.
    bool correct = true;
    for (uint index = lane; index < INPUT_SIZE * OUTPUT_SIZE; index += ThreadCount)
    {
        uint i = index / INPUT_SIZE;
        uint j = index % INPUT_SIZE;
        float expected = float((i % 5 + 1) * (j % 7 + 1))
            * float(ThreadCount * (ThreadCount + 1) / 2) / 2048.0;
        correct = correct && abs(float(gradients[index]) - expected) <= max(0.002, expected * 0.005);
    }
    correct = WaveActiveAllTrue(correct);
    // Report failures from every subgroup, not just subgroup zero.
    if (lane == 0)
        result[0] = 1;
    AllMemoryBarrierWithGroupSync();
    if (!correct && WaveIsFirstLane())
        InterlockedAnd(result[0], 0u);
}
// BUFFER: 1
