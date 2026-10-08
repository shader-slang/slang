// Test that a bit_cast of an array of resource handles reports E41205 unless the cast is
// the identity (issue #13480). Resource-type legalization keeps such an array whole, so
// `lowerOpaqueBitCast` leaves the cast alone and `lowerBitCast`, which cannot build a
// handle from bytes, rejects a destination that holds one. Master aborts with E99997.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute

uniform StructuredBuffer<uint> gBuffers[2];
RWStructuredBuffer<uint64_t> bitsBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    StructuredBuffer<float> floats[2] = bit_cast<StructuredBuffer<float>[2], StructuredBuffer<uint>[2]>(gBuffers);
//CHECK:                                                                                               ^ E41205
//CHECK:                                                                                               ^ cannot bit_cast 'array<StructuredBuffer<uint>,2>' to 'array<StructuredBuffer<float>,2>'
//CHECK:                                                                                               ^ is supported when each handle maps
    outputBuffer[0] = floats[1][0];

    uint64_t bits[2] = { bitsBuffer[0], bitsBuffer[1] };
    Texture2D<float> textures[2] = bit_cast<Texture2D<float>[2], uint64_t[2]>(bits);
//CHECK:                                                                     ^ E41205
//CHECK:                                                                     ^ cannot bit_cast 'array<uint64_t,2>' to 'array<Texture2D,2>'
//CHECK:                                                                     ^ is supported when each handle maps
    outputBuffer[1] = textures[1].Load(int3(0));
}
