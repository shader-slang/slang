//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute, nvvm):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute, nvvm):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// This is also exercised with repeated dispatch profiling by test-cuda-dispatch-profile.py.
// Each launch must start with the original values, including unmarked mutable inputs and counters.
//TEST_INPUT: set values = ubuffer(data=[10 20], stride=4)
RWStructuredBuffer<uint> values;
//TEST_INPUT: set smallValues = ubuffer(data=[1 2 3 4], stride=1)
RWStructuredBuffer<uint8_t> smallValues;
//TEST_INPUT: set halfValues = ubuffer(data=[100 200], stride=2)
RWStructuredBuffer<uint16_t> halfValues;
//TEST_INPUT: set readonlyValues = ubuffer(data=[7], stride=4)
StructuredBuffer<uint> readonlyValues;
//TEST_INPUT: set appended = out ubuffer(data=[0 0 0 0], stride=4, counter=1)
AppendStructuredBuffer<uint> appended;
//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> output;
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content=zero):name textureValue
RWTexture2D<uint> textureValue;

static uint accumulator = 9;

[numthreads(1, 1, 1)]
void computeMain()
{
    values[0] += readonlyValues[0];
    smallValues[2] += 1;
    halfValues[1] += 2;
    appended.Append(values[0]);
    output[0] += values[0];
    output[1] = smallValues[2];
    output[2] = halfValues[1];
    textureValue[uint2(1, 1)] += 3;
    output[3] = ++accumulator + textureValue[uint2(1, 1)];
}

// CHECK: type: uint32_t
// CHECK-NEXT: 0
// CHECK-NEXT: 17
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: type: uint32_t
// CHECK-NEXT: 17
// CHECK-NEXT: 4
// CHECK-NEXT: 202
// CHECK-NEXT: 13
