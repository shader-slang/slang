//TEST:COMPILE: tests/language-feature/types/matrix-layout-overload-library.slang -o tests/language-feature/types/matrix-layout-overload-library.slang-module
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Regression for shader-slang/slang#13383 across a precompiled module: overloads that differ only
// in matrix layout keep distinct mangled names in the serialized module, so each call links to its
// own body.

import matrix_layout_overload_library;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    var rowArray : row_major float2x3[2] = {};
    var columnArray : column_major float2x3[2] = {};
    outputBuffer[0] = pick(rowArray);
    outputBuffer[1] = pick(columnArray);
}

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
