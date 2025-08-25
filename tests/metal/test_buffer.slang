// Test that Buffer<T> maps to texture_buffer<uint, access::read> in Metal

//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -output-using-type

// FIXME: No test input is provided for inputBuffer because TextureBuffer isn't
// handled by the metal backend of the slang-test infrastructure.

// METAL: texture_buffer<uint, access::read> inputBuffer_{{.*}}
Buffer<uint> inputBuffer;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    uint idx = dtid.x;
    // Load values from the buffer to verify correct access
    outputBuffer[idx] = inputBuffer.Load(idx);
    // BUF: 0
    // BUF: 0
    // BUF: 0
    // BUF: 0
}
