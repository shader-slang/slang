//TEST:SIMPLE_EX(filecheck=SPV): -lang glsl tests/glsl/array-nested-type-5870.slang -target spirv-asm -stage compute -entry main
//TEST:SIMPLE_EX(filecheck=GLSL): -lang glsl tests/glsl/array-nested-type-5870.slang -target glsl -stage compute -entry main

// GLSL reads a type-expression bracket chain with the first bracket as the
// outermost dimension, matching the declaration spelling, so the GLSL-style
// nested array constructor `int[2][4](a, b)` must compile and produce the same
// 2x4 table the declaration spelling gives (issue #5870). In Slang and HLSL
// source mode a type expression instead follows specification/types-array.md
// and binds the last bracket outermost, which the existing suite pins.

RWStructuredBuffer<int> outputBuffer;

const int TOP[4] = int[4](0, 0, 1, 0);
const int BOT[4] = int[4](0, 1, 0, 1);

// The issue's constructor spelling, and the declaration spelling it must
// agree with; the 2x4 table is BOT above TOP.
const int LUT[2][4] = int[2][4](TOP, BOT);
const int DECL[2][4] = { TOP, BOT };

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // A transposed reading of either spelling would fail to compile the
    // constructor or index out of bounds here.
    outputBuffer[0] = LUT[1][3];
    outputBuffer[1] = LUT[1][0];
    outputBuffer[2] = DECL[1][1];
}
//SPV: %int_0 = OpConstant %int 0
//SPV: %int_1 = OpConstant %int 1
//GLSL: _data[uint(0)] = 1;
//GLSL: _data[uint(1)] = 0;
//GLSL: _data[uint(2)] = 1;
