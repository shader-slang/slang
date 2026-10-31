// Regression for shader-slang/slang#13375: the matrix layout on a traditional-syntax pointer
// parameter applies to the pointee, the same as on a struct field, so a `row_major float2x3*`
// field can be passed to a `row_major float2x3*` parameter.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry main -stage compute -matrix-layout-column-major

struct Params
{
    row_major float2x3* matrix;
}
ConstantBuffer<Params> params;

RWStructuredBuffer<float> outputBuffer;

float readEntry(row_major float2x3* m)
{
    return (*m)[0][1];
}

// The pointee is stored row by row (`natural`), not with the column-major default.
// SPV-NOT: _MatrixStorage_float2x3_ColMajor
// SPV: OpTypePointer PhysicalStorageBuffer %_MatrixStorage_float2x3natural
// SPV-NOT: _MatrixStorage_float2x3_ColMajor

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = readEntry(params.matrix);
}
