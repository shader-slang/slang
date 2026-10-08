// Test that on CUDA a bit_cast from plain data to a struct holding a texture reports
// a diagnostic instead of an internal error (issue #13480).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute

struct A
{
    Texture2D<float> tex;
}

uniform uint2 gBits;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    A a = bit_cast<A, uint2>(gBits);
//CHECK:                    ^ E41205
//CHECK:                    ^ cannot bit_cast 'vector<uint,2>' to 'A'
//CHECK:                    ^ has no bit representation here
    outputBuffer[0] = a.tex.Load(int3(0));
}
