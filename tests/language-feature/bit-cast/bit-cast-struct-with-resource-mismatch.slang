// Test that a bit_cast whose resource fields do not line up position-for-position
// reports a diagnostic instead of an internal error (issue #13480).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

struct A
{
    StructuredBuffer<float> data;
    uint offset;
}

struct B
{
    uint offset;
    StructuredBuffer<float> data;
}

uniform A gA;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    B b = bit_cast<B, A>(gA);
//CHECK:                ^ E41205
//CHECK:                ^ cannot bit_cast 'A' to 'B'
//CHECK:                ^ has no bit representation here
    outputBuffer[0] = b.data[b.offset];
}
