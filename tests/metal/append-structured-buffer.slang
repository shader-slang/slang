//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

//TEST_INPUT:set inBuffer = ubuffer(data=[1 2 3 4], stride=4)
RWStructuredBuffer<int> inBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4, counter=0):out,name=outputBuffer
AppendStructuredBuffer<int> outputBuffer;

// METAL: atomic_fetch_add_explicit(((atomic_int device*)
// METAL: AppendStructuredBuffer_Append_0(outputBuffer_elements{{.*}}, outputBuffer_counter

// METALLIB: define void @computeMain
// METALLIB: call {{.*}}.atomic.global.add.s.i32

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
