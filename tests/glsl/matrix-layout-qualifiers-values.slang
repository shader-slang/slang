//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -emit-spirv-via-glsl

// Both blocks hold the same GLSL mat2x3 with columns (1,2,3) and (4,5,6): `RowBlock` stores it
// row by row under a block-level `row_major`, `ColumnBlock` column by column (the GLSL default).

#version 450
layout(local_size_x = 1) in;

//TEST_INPUT:cbuffer(data=[1.0 4.0 0.0 0.0  2.0 5.0 0.0 0.0  3.0 6.0 0.0 0.0]):name=RowBlock
layout(std140, row_major, binding = 0) uniform RowBlock { mat2x3 rowMatrix; };

//TEST_INPUT:cbuffer(data=[1.0 2.0 3.0 0.0  4.0 5.0 6.0 0.0]):name=ColumnBlock
layout(std140, binding = 1) uniform ColumnBlock { mat2x3 columnMatrix; };

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
layout(std430, binding = 2) buffer Out { int o[]; } outputBuffer;

void computeMain()
{
    for (int c = 0; c < 2; c++)
    {
        for (int r = 0; r < 3; r++)
        {
            outputBuffer.o[c * 3 + r] = int(rowMatrix[c][r]);
            outputBuffer.o[6 + c * 3 + r] = int(columnMatrix[c][r]);
        }
    }
}

// BUF: 1
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 4
// BUF-NEXT: 5
// BUF-NEXT: 6
// BUF-NEXT: 1
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 4
// BUF-NEXT: 5
// BUF-NEXT: 6
