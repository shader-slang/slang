//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-shaderobj -output-using-type -dx12

// Regression test for shader-slang/slangpy#827: structs containing void
// fields (e.g. from zero-length arrays) inside a ParameterBlock must not
// crash during resource type legalization.
//
// When a struct has both resource fields (special) and void fields, the
// legalization pass pair-splits the struct. Previously, void fields were
// omitted from the PairInfo, causing "didn't find tuple element" crashes
// when code extracted those fields from the pair-split value.

struct Data
{
    RWStructuredBuffer<float> buffer;
    int shape[0];
    int strides[0];
    uint offset;
};

int computeIndex(int strides[0], uint offset)
{
    return int(offset);
}

void writeData(Data d, uint idx, float value)
{
    int index = computeIndex(d.strides, d.offset);
    d.buffer[index + idx] = value;
}

//TEST_INPUT: set pb = new{ out ubuffer(data=[0.0 0.0 0.0 0.0], stride=4, format=R32_FLOAT), 0 }
ParameterBlock<Data> pb;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    writeData(pb, tid.x, float(tid.x + 1) * 10.0);
}

// CHECK: 10.0
// CHECK: 20.0
// CHECK: 30.0
// CHECK: 40.0
