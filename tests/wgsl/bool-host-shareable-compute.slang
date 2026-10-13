//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -wgpu -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -output-using-type

// Reads and writes `bool` data stored in uniform and storage buffers
// (shader-slang/slang#13590).

struct Params
{
    bool flag;
    bool2 pair;
    bool arr[2];
};

// std140 words: flag @0, pair @2..3, arr[0] @4, arr[1] @8 (48 bytes).
//TEST_INPUT:set params = cbuffer(data=[1 0 0 1 0 0 0 0 1 0 0 0])
ConstantBuffer<Params> params;

struct Elem
{
    float x;
    bool f;
};

//TEST_INPUT:set elems = ubuffer(data=[0 1 0 0], stride=8)
RWStructuredBuffer<Elem> elems;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    elems[1].f = !elems[0].f;

    // CHECK: 1
    outputBuffer[0] = params.flag ? 1 : 0;
    // CHECK: 0
    outputBuffer[1] = params.pair.x ? 1 : 0;
    // CHECK: 1
    outputBuffer[2] = params.pair.y ? 1 : 0;
    // CHECK: 0
    outputBuffer[3] = params.arr[0] ? 1 : 0;
    // CHECK: 1
    outputBuffer[4] = params.arr[1] ? 1 : 0;
    // CHECK: 1
    outputBuffer[5] = elems[0].f ? 1 : 0;
    // CHECK: 0
    outputBuffer[6] = elems[1].f ? 1 : 0;
}
