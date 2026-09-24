//TEST:COMPILE: tests/serialization/pragma-pack-matrix-roundtrip.slang -o tests/serialization/pragma-pack-matrix-roundtrip.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module tests/serialization/pragma-pack-matrix-roundtrip.slang-module

// RowMajor is enum value 1. Reading the serialized module must recover that concrete layout rather
// than an unresolved default (enum value 0).
// CHECK: field(%value, Mat(Float, 2 : Int, 3 : Int, 1 : Enum(Int)))

#pragma pack_matrix(row_major)

struct Params
{
    float2x3 value;
}

StructuredBuffer<Params> input;
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = input[0].value[0][0];
}
