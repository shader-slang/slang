// Test bit_casts whose resource fields line up but whose ordinary fields sit at
// different offsets (issue #13480). SPIR-V gives the resource field no bit
// representation, so each cast is rejected.
// bit-cast-struct-with-resource-layout-mismatch-cuda.slang covers the first cast on CUDA.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// `b` follows a 1-byte field in `A` but is 4-byte aligned in `B`, so it starts at a different
// offset (9 and 12 on SPIR-V, where the buffer field takes 8 bytes).
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
//CHECK:                ^ is supported when each handle maps
    outputBuffer[0] = b.b;

    D d = bit_cast<D, C>(gC);
//CHECK:                ^ E41205
//CHECK:                ^ cannot bit_cast 'C' to 'D'
//CHECK:                ^ is supported when each handle maps
    outputBuffer[1] = d.data[d.offset];
}
