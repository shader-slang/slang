//TEST:SIMPLE(filecheck=STRIDES): -target spirv-asm -entry computeMain

// Resource/layout context for issue #5870: a StructuredBuffer whose element
// type is a type-position nested array must pack like the declaration
// spelling `int x[2][4]` — element type Array<Array<int,4>,2>: rows of 4
// ints (inner rows stride 16 bytes), buffer elements 32 bytes. A transposed
// reading of `int[2][4]` would make rows of 2 ints and stride the outer
// array at 8.
StructuredBuffer<int[2][4]> nestedIn;
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = nestedIn[1][1][3];
}

//STRIDES: OpDecorate {{%[^ ]*}} ArrayStride 16
//STRIDES-NOT: OpDecorate {{%[^ ]*}} ArrayStride 8
