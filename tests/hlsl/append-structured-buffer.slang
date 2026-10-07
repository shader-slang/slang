//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-dx12 -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -output-using-type -xslang -DPOINTER_ONLY_BUFFER
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -output-using-type

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// To check that our counter-initialization works correctly, set the initial
// counter to 1 instead of 0
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4, counter=1):out,name=outputBuffer
AppendStructuredBuffer<int> outputBuffer;

//TEST_INPUT:set inBuffer = ubuffer(data=[1 2 3 4], stride=4)
RWStructuredBuffer<int> inBuffer;

// Make sure the bindings are correct. outputBuffer should take two slots, and inBuffer
// should be at binding 3, after the uniform buffer containing bufferCount.
// CHECK: OpDecorate %inBuffer Binding 3

//TEST_INPUT:set bufferCount = 8
uniform uint bufferCount;

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    int g = inBuffer[i];
    outputBuffer.Append(g);

    GroupMemoryBarrier();

    uint numStructs, stride;
    // CUDA buffer references have no length; the host supplies it explicitly.
#ifdef POINTER_ONLY_BUFFER
    numStructs = bufferCount;
#else
    outputBuffer.GetDimensions(numStructs, stride);
#endif
    if(i == 0)
        outputBuffer.Append(int(numStructs));

    // BUF: type: int32_t
    // Never assigned, as we set the initial counter to 1
    // BUF: 0

    // The values from inBuffer in any order
    // BUF-DAG: 1
    // BUF-DAG: 3
    // BUF-DAG: 2
    // BUF-DAG: 4

    // The total size of the AppendStructuredBuffer (from GetDimensions)
    // BUF: 8

    // Never assigned
    // BUF: 0
    // BUF: 0
}
