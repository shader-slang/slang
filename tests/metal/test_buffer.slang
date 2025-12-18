// Test that Buffer<T> maps to texture_buffer<uint, access::read> in Metal

//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal
//TEST:SIMPLE(filecheck=METALLIB): -stage compute -entry computeMain -target metallib

// No COMPARE_COMPUTE test; currently the test framework can't setup a Buffer<>
// TODO: github issue #8456

// METAL: texture_buffer<uint, access::read> inputBuffer_{{.*}}
Buffer<uint> inputBuffer;

RWStructuredBuffer<uint> outputBuffer;

// METALLIB: @computeMain
[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    uint idx = dtid.x;
    // Load values from the buffer to verify correct access
    outputBuffer[idx] = inputBuffer.Load(idx);
}
