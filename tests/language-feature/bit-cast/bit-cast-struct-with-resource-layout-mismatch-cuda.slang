// Test that on CUDA, where a resource field is an ordinary value, a bit_cast whose
// ordinary fields sit at different offsets (`b` is at offset 17 in `A` and offset 20
// in `B`) is rejected as a size mismatch with a single error (issue #13480). Resource
// fields that line up position-for-position do not by themselves make such a cast compile.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute

struct A
{
    StructuredBuffer<uint> data;
    uint8_t a;
    uint8_t[4] b;
}

struct B
{
    StructuredBuffer<uint> data;
    uint8_t a;
    uint b;
}

uniform A gA;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    B b = bit_cast<B, A>(gA);
//CHECK:                ^ E41202
//CHECK:                ^ invalid to bit_cast differently sized types
    outputBuffer[0] = b.b;
}
