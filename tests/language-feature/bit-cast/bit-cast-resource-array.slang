// Test that a bit_cast of an array of resource handles, which resource-type
// legalization keeps whole, is left to the target (issue #13480). An identity cast
// compiles on every target, and on CUDA/CPU, where a handle is an ordinary value, a cast
// to plain data keeps its byte semantics.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=BYTES): -target cuda -entry computeMain -stage compute -DTEST_BYTES
//TEST:SIMPLE(filecheck=BYTES): -target cpp -entry computeMain -stage compute -DTEST_BYTES

// CHECK: result code = 0
// BYTES: result code = 0
// BYTES: slang_bit_cast<{{.*}}>({{.*}}gTextures

uniform Texture2D<float> gTextures[2];
RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<uint64_t> bitsBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Texture2D<float> textures[2] = bit_cast<Texture2D<float>[2], Texture2D<float>[2]>(gTextures);
    outputBuffer[0] = textures[1].Load(int3(0));

#ifdef TEST_BYTES
    uint64_t[2] bits = bit_cast<uint64_t[2], Texture2D<float>[2]>(gTextures);
    bitsBuffer[0] = bits[1];
#endif
}
