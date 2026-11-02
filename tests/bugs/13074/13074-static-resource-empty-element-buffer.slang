// CPU-style targets run empty-type legalization without running resource-type legalization. This
// buffer's element type changes during that step, but the selected global remains one value for
// resource-global replacement.
//
//TEST:SIMPLE: -target cpp -entry testMain -stage compute -validate-ir -o 13074-static-resource-empty-element-buffer.cpp

struct Empty
{}

StructuredBuffer<Empty> inputBuffer;
RWStructuredBuffer<uint> output;
static StructuredBuffer<Empty> cachedBuffer = inputBuffer;

[numthreads(1, 1, 1)]
void testMain()
{
    uint elementCount;
    uint elementStride;
    cachedBuffer.GetDimensions(elementCount, elementStride);
    output[0] = elementCount + elementStride;
}
