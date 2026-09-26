//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:ubuffer(data=[2 1], stride=4):name=inputs
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputs
// CHECK: 432
// CHECK-NEXT: 9
// CHECK-NEXT: 876
// CHECK-NEXT: 31
// CHECK-NEXT: 654
// CHECK-NEXT: 10
// CHECK-NEXT: 765
// CHECK-NEXT: 11

struct Payload
{
    float3 value;
    float sentinel;
};
struct Nested
{
    Payload payload;
    float3 values[2];
    int3 integers;
    float4 control;
};

[noinline]
float3 readPayload(__constref Payload payload)
{
    return payload.value;
}
[noinline]
float readSentinel(__constref Payload payload)
{
    return payload.sentinel;
}
[noinline]
float3 readNested(__constref Nested value)
{
    return value.payload.value;
}
[noinline]
float readLane(__constref Nested value, uint lane)
{
    return value.values[1][lane] + value.payload.sentinel;
}
[noinline]
float3 readArray(__constref Nested value, uint index)
{
    return value.values[index];
}
[noinline]
int3 readIntegers(__constref Nested value)
{
    return value.integers;
}
[noinline]
float4 readControl(__constref Nested value)
{
    return value.control;
}
[noinline]
void mutate(inout Payload payload)
{
    payload.value += float3(3, 3, 3);
    payload.sentinel += 2;
}
uint encode(float3 value)
{
    return uint(value.x + 10 * value.y + 100 * value.z);
}
StructuredBuffer<uint> inputs;
RWStructuredBuffer<uint> outputs;
[numthreads(1, 1, 1)]
void computeMain()
{
    float seed = float(inputs[0]);
    Payload local = {float3(seed, seed + 1, seed + 2), 9};
    outputs[0] = encode(readPayload(local));
    outputs[1] = uint(readSentinel(local));
    Nested nested = {{float3(6, 7, 8), 11}, {float3(10, 11, 12), float3(20, 21, 22)},
                     int3(4, 5, 6), float4(1, 2, 3, 4)};
    outputs[2] = encode(readNested(nested));
    outputs[3] = uint(readLane(nested, inputs[1] - 1));
    outputs[4] = encode(float3(readIntegers(nested)));
    outputs[5] = uint(dot(readControl(nested), float4(1, 1, 1, 1)));
    // A borrowed read must not change the writable native layout used by the same payload.
    mutate(local);
    outputs[6] = encode(readPayload(local));
    outputs[7] = uint(readArray(nested, inputs[1] - 1).y);
}
