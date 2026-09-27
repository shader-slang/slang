// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[1.0 2.0 4.0 8.0],stride=16):name=inputs
StructuredBuffer<float4> inputs;
//TEST_INPUT:ubuffer(data=[31337 31337 31337 31337 31337 31337 31337 31337 31337 31337 31337 31337 31337 31337 31337],stride=4):out,name=results
RWStructuredBuffer<int> results;

// A single source call must remain one evaluation even when its result supplies three lanes.
[noinline]
half3 replacement(half4 value, inout uint calls)
{
    ++calls;
    return -value.zwx;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    half4 original = half4(inputs[0]);
    half4 overlap = original;
    overlap.xyz = -overlap.zwx;
    half4 noncontiguous = original;
    noncontiguous.xw = noncontiguous.wy;
    uint calls = 0;
    half4 effectful = original;
    effectful.xyz = replacement(effectful, calls);
    for (uint lane = 0; lane < 4; ++lane)
    {
        results[lane] = int(overlap[lane]);
        results[4 + lane] = int(noncontiguous[lane]);
        results[8 + lane] = int(effectful[lane]);
    }
    results[12] = int(calls);
    results[13] = 3;
    // Word14 remains an untouched guard. All values are exactly representable in half.
}

// CHECK: type: int
// CHECK-NEXT: -4{{$}}
// CHECK-NEXT: -8{{$}}
// CHECK-NEXT: -1{{$}}
// CHECK-NEXT: 8{{$}}
// CHECK-NEXT: 8{{$}}
// CHECK-NEXT: 2{{$}}
// CHECK-NEXT: 4{{$}}
// CHECK-NEXT: 2{{$}}
// CHECK-NEXT: -4{{$}}
// CHECK-NEXT: -8{{$}}
// CHECK-NEXT: -1{{$}}
// CHECK-NEXT: 8{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 3{{$}}
// CHECK-NEXT: 31337{{$}}
