// Test bit_casts whose resource fields line up but whose ordinary fields sit at
// different offsets (issue #13480). SPIR-V gives the resource field no bit
// representation, so each cast is rejected.
// bit-cast-struct-with-resource-layout-mismatch-cuda.slang covers the first cast on CUDA.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// `b` is at offset 17 in `A` and offset 20 in `B`.
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

// `offset` is placed at 16 by `[[vk::offset]]` in `C` and at 8 in `D`.
struct C
{
    StructuredBuffer<uint> data;
    [[vk::offset(16)]]
    uint offset;
}

struct D
{
    StructuredBuffer<uint> data;
    uint offset;
}

uniform A gA;
uniform C gC;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    B b = bit_cast<B, A>(gA);
//CHECK:                ^ E41205
//CHECK:                ^ cannot bit_cast 'A' to 'B'
    outputBuffer[0] = b.b;

    D d = bit_cast<D, C>(gC);
//CHECK:                ^ E41205
//CHECK:                ^ cannot bit_cast 'C' to 'D'
    outputBuffer[1] = d.data[d.offset];
}
