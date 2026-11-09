// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST:COMPILE: tests/language-feature/generics/type-pack-index-library.slang -o tests/language-feature/generics/type-pack-index-library.slang-module
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

import type_pack_index_library;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = typeIndexAsGenericArgument<float, int, float, bool>(); // CHECK: 1
    outputBuffer[1] = expandedTypeIndex<bool, int, float, bool>(); // CHECK: 2
    TypeIndexedArray<bool, int, float, bool> array = { 7, 11, 13 };
    outputBuffer[2] = array[2]; // CHECK: 13
}
