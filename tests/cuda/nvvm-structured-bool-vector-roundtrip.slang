// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

struct Item
{
    bool enabled;
    float3 value;
};
struct Payload
{
    Item items[2];
    bool ready;
    float3 tail;
};

// CUDA storage is three 16-byte groups: one Boolean byte, padding, then three floats.
//TEST_INPUT: ubuffer(data=[1 1065353216 1073741824 1077936128 0 1082130432 1084227584 1086324736 1 1088421888 1090519040 1091567616], stride=48):name=sourceBuffer
StructuredBuffer<Payload> sourceBuffer;
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=48):name=destination
RWStructuredBuffer<Payload> destination;
//TEST_INPUT: ubuffer(data=[99 99 99 99 99 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

uint encode(float3 value)
{
    return uint(value.x + 10 * value.y + 100 * value.z);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Payload value = sourceBuffer[0];
    outputBuffer[0] = uint(value.items[0].enabled) + 2 * uint(value.items[1].enabled) +
        4 * uint(value.ready);
    value.items[0].enabled = !value.items[0].enabled;
    value.items[1].enabled = !value.items[1].enabled;
    value.ready = !value.ready;
    value.tail += float3(10.0);
    destination[0] = value;
    // This single-thread observation crosses an actual global-memory effect boundary.
    DeviceMemoryBarrier();
    Payload loaded = destination[0];
    outputBuffer[1] = uint(loaded.items[0].enabled) + 2 * uint(loaded.items[1].enabled) +
        4 * uint(loaded.ready);
    outputBuffer[2] = encode(loaded.items[0].value);
    outputBuffer[3] = encode(loaded.items[1].value);
    outputBuffer[4] = encode(loaded.tail);
}

// CHECK: type: uint
// CHECK-NEXT: 5{{$}}
// CHECK-NEXT: 2{{$}}
// CHECK-NEXT: 321{{$}}
// CHECK-NEXT: 654{{$}}
// CHECK-NEXT: 2097{{$}}
// CHECK-NEXT: 3735928559{{$}}
