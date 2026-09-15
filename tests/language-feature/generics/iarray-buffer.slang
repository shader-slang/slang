//TEST(smoke):SIMPLE:

T readFirst<T, TArray : IArray<T>>(TArray values)
{
    return values[0];
}

Buffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = readFirst(inputBuffer) + float(inputBuffer.getCount());
}
