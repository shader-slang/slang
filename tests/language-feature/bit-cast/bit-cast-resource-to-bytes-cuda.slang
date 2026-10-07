// Test that on CUDA, where a texture handle is an ordinary 64-bit value, a bit_cast from
// a struct holding a texture to plain data keeps its byte semantics (issue #13480).
// This test already passes on master; it pins that the opaque-aggregate bit_cast
// rewrite leaves byte-level casts alone on targets without resource legalization.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

struct A
{
    Texture2D<float> tex;
}

uniform A gA;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: slang_bit_cast<{{.*}}>({{.*}}tex
    uint2 u = bit_cast<uint2, A>(gA);
    outputBuffer[0] = u.x;
}
