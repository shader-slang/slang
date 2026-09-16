//TEST(smoke):SIMPLE:

T readAt<T, TArray : IArray<T>>(TArray values, uint index)
{
    return values[index];
}

Buffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = readAt(inputBuffer, 0) + float(inputBuffer.getCount());
}
