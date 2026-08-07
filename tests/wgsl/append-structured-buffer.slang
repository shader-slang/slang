//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -output-using-type
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

//TEST_INPUT:set inBuffer = ubuffer(data=[1 2 3 4], stride=4)
RWStructuredBuffer<int> inBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4, counter=0):out,name=outputBuffer
AppendStructuredBuffer<int> outputBuffer;

// WGSL: array<atomic<i32>>
// WGSL: atomicAdd

// BUF: type: int32_t
// BUF-DAG: 1
// BUF-DAG: 2
// BUF-DAG: 3
// BUF-DAG: 4
// BUF: 0
// BUF: 0
// BUF: 0
// BUF: 0

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outputBuffer.Append(inBuffer[i]);
}
