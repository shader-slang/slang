//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3

struct Inner { uint8_t tag; uint value; }
struct Outer { uint8_t tag; Inner inner; }

[noinline]
float readLane(float values[256], uint8_t index)
{
    return values[index];
}

[noinline]
Outer readRecord(Outer values[3], uint index)
{
    return values[index];
}

//TEST_INPUT:ubuffer(data=[200 2 32], stride=4):name=parameters
StructuredBuffer<uint> parameters;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;
// CHECK: 200
// CHECK-NEXT: 6896
// CHECK-NEXT: 37
// CHECK-NEXT: 79
[numthreads(1, 1, 1)]
void computeMain()
{
    float values[256];
    for (uint i = 0; i < 256; ++i)
        values[i] = float(i);
    uint8_t index = uint8_t(parameters[0]);
    outputs[0] = uint(readLane(values, index));
    float total = 0;
    for (uint i = 0; i < parameters[2]; ++i)
    {
        total += readLane(values, index);
        values[uint(index)] += 1;
    }
    outputs[1] = uint(total); // 32 * 200 + 32 * 31 / 2

    Outer records[3] = {
        {uint8_t(1), {uint8_t(2), 3}},
        {uint8_t(4), {uint8_t(5), 6}},
        {uint8_t(7), {uint8_t(11), 19}}
    };
    Outer before = readRecord(records, parameters[1]);
    records[parameters[1]].inner.value = 61;
    Outer after = readRecord(records, parameters[1]);
    outputs[2] = uint(before.tag) + uint(before.inner.tag) + before.inner.value;
    outputs[3] = uint(after.tag) + uint(after.inner.tag) + after.inner.value;
}
