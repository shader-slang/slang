//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv-asm -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_OUTPUT): -cpu -compute -shaderobj -output-using-type

// Regression test for #10212.
// Matrix swizzles passed as out/inout arguments must write back to the selected
// matrix element, including constant-indexed elements of multi-element swizzles.
// Vector swizzles use the same l-value write-back path, so keep a vector case
// here to guard both branches.
//
// `_41` and `_41_32[0]` select row 3, column 0. `_41_32[1]` selects row 2,
// column 1. The checks below make sure call write-back fixup preserves those
// l-values instead of materializing the swizzle as an r-value.

// CHECK_HLSL: writeOut_0(m_0[int(3)][int(0)], 7.0f);
// CHECK_HLSL: writeOut_0(m_0[int(3)][int(0)], 11.0f);
// CHECK_HLSL: addInOut_0(m_0[int(2)][int(1)], 5.0f);
// CHECK_HLSL: writeOut_0(v_0[int(1)], 13.0f);
// CHECK_HLSL: addInOut_0(v_0[int(0)], 17.0f);
// CHECK_HLSL: outputBuffer_0[int(0)] = m_0[int(3)][int(0)];
// CHECK_HLSL: outputBuffer_0[int(1)] = m_0[int(2)][int(1)];

// CHECK_SPV-DAG: %[[ROW3:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Function_v4float %m %int_3
// CHECK_SPV-DAG: %[[ELEM30:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Function_float %[[ROW3]] %int_0
// CHECK_SPV: OpStore %[[ELEM30]] %float_11
// CHECK_SPV-DAG: %[[ROW2:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Function_v4float %m %int_2
// CHECK_SPV-DAG: %[[ELEM21:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Function_float %[[ROW2]] %int_1
// CHECK_SPV: OpStore %[[ELEM21]]

// CHECK_OUTPUT: 11.0
// CHECK_OUTPUT-NEXT: 5.0
// CHECK_OUTPUT-NEXT: 11.0
// CHECK_OUTPUT-NEXT: 5.0
// CHECK_OUTPUT-NEXT: 17.0
// CHECK_OUTPUT-NEXT: 13.0

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

void writeOut(out float x, float value)
{
    x = value;
}

void addInOut(inout float x, float value)
{
    x += value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float4x4 m = float4x4(0);

    writeOut(m._41, 7.0);
    writeOut(m._41_32[0], 11.0);
    addInOut(m._41_32[1], 5.0);

    float4 v = float4(0);
    writeOut(v.yx[0], 13.0);
    addInOut(v.yx[1], 17.0);

    outputBuffer[0] = m[3][0];
    outputBuffer[1] = m[2][1];
    outputBuffer[2] = m._41_32[0];
    outputBuffer[3] = m._41_32[1];
    outputBuffer[4] = v.x;
    outputBuffer[5] = v.y;
}
