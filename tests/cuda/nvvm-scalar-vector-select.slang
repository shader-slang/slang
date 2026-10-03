// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute, nvvm):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute, nvvm):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvrtc -Xslang -O3

//TEST_INPUT: set values = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=16)
//TEST_INPUT: set choices = ubuffer(data=[0 1 0 1], stride=4)
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
StructuredBuffer<float4> values;
StructuredBuffer<uint> choices;
RWStructuredBuffer<uint> outputBuffer;

// Both alternatives come from buffers so the compiler cannot replace selection with a constant.
// Expected lanes are authored independently: the first vector is (1,2,3,4), the second (5,6,7,8).
bool checkSelection<T : __BuiltinArithmeticType, let N : int>(
    bool condition, vector<float, N> first, vector<float, N> second)
{
    vector<T, N> firstValue;
    vector<T, N> secondValue;
    [ForceUnroll]
    for (int lane = 0; lane < N; ++lane)
    {
        firstValue[lane] = T(int(first[lane]));
        secondValue[lane] = T(int(second[lane]));
    }
    vector<T, N> selected = select(condition, firstValue, secondValue);
    bool correct = true;
    [ForceUnroll]
    for (int lane = 0; lane < N; ++lane)
    {
        if (condition)
            correct = correct && selected[lane] == T(lane + 1);
        else
            correct = correct && selected[lane] == T(lane + 5);
    }
    return correct;
}

bool checkWidths<T : __BuiltinArithmeticType>(bool condition, float4 first, float4 second)
{
    return checkSelection<T, 2>(condition, first.xy, second.xy) &&
           checkSelection<T, 3>(condition, first.xyz, second.xyz) &&
           checkSelection<T, 4>(condition, first, second);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bool condition = choices[tid.x] != 0;
    float4 first = values[0];
    float4 second = values[1];
    bool correct = checkWidths<int8_t>(condition, first, second) &&
                   checkWidths<uint8_t>(condition, first, second) &&
                   checkWidths<int16_t>(condition, first, second) &&
                   checkWidths<uint16_t>(condition, first, second) &&
                   checkWidths<int>(condition, first, second) &&
                   checkWidths<uint>(condition, first, second) &&
                   checkWidths<int64_t>(condition, first, second) &&
                   checkWidths<uint64_t>(condition, first, second) &&
                   checkWidths<half>(condition, first, second) &&
                   checkWidths<float>(condition, first, second) &&
                   checkWidths<double>(condition, first, second);

    bool4 firstMask = first > float4(0, 3, 2, 5);
    bool4 secondMask = second > float4(6, 5, 8, 7);
    bool2 selected2 = select(condition, firstMask.xy, secondMask.xy);
    bool3 selected3 = select(condition, firstMask.xyz, secondMask.xyz);
    bool4 selected4 = select(condition, firstMask, secondMask);
    if (condition)
        correct = correct && all(selected2 == bool2(true, false)) &&
                  all(selected3 == bool3(true, false, true)) &&
                  all(selected4 == bool4(true, false, true, false));
    else
        correct = correct && all(selected2 == bool2(false, true)) &&
                  all(selected3 == bool3(false, true, false)) &&
                  all(selected4 == bool4(false, true, false, true));
    outputBuffer[tid.x] = correct ? 1 : 0;

    // CHECK: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
}
