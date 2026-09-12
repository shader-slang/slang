layout(local_size_x = 1) in;

layout(binding = 0) buffer OutputBuffer
{
    uint value;
} outputBuffer;

layout(binding = 1, offset = 12) uniform atomic_uint counter;

// Expose an operation that keeps the counter and its GLSL-only type live across serialization.
void incrementSeparatelyCompiledCounter()
{
    outputBuffer.value = atomicCounterIncrement(counter);
}
