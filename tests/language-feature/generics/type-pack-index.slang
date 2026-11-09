// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type -xslang -O0 -xslang -g2 -xslang -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -d3d12 -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL): -entry computeMain -target hlsl -O0 -g2 -validate-ir

import type_pack_index_library;

struct First { int value; }
struct Second { int value; }
typealias Alias = Second;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = typeIndex<int, int, float, bool>(); // CHECK: 0
    outputBuffer[1] = typeIndex<float, int, float, bool>(); // CHECK: 1
    outputBuffer[2] = typeIndex<bool, int, float, bool>(); // CHECK: 2
    outputBuffer[3] = forwardedTypeIndex<Alias, First, Second>(); // CHECK: 1
    outputBuffer[4] = typeIndexAsGenericArgument<float, int, float, bool>(); // CHECK: 1
    TypeIndexedArray<float, int, float, bool> array = { 11, 17 };
    outputBuffer[5] = array[1]; // CHECK: 17
    outputBuffer[6] = expandedTypeIndex<bool, int, float, bool>(); // CHECK: 2
    outputBuffer[7] = trimmedTypeIndex<bool, int, float, bool>(); // CHECK: 1
    outputBuffer[8] = valueDependentTypeIndex<3>(); // CHECK: 1
    outputBuffer[9] = typeIndex<TypeIndexOuter<float>.Inner,
        TypeIndexOuter<int>.Inner, TypeIndexOuter<float>.Inner>(); // CHECK: 1
    outputBuffer[10] = associatedTypeIndex<TypeIndexAssociated, int, float>(); // CHECK: 1
    outputBuffer[11] = __index_of_type_in_pack<float, expand each Tuple<int, float, bool>>; // CHECK: 1
    static_assert(__index_of_type_in_pack<float, expand each Tuple<int, float, bool>> == 1,
        "Type-pack lookup must be a compile-time integer.");
}

// Even at -O0, the specialized lookup body is just a constant return.
// HLSL: int expandedTypeIndex_0()
// HLSL: return int(2);
// HLSL: void computeMain()
// HLSL: int  array_0[int(2)]
