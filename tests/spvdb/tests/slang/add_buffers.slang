// Simple compute shader: reads from two input buffers, writes sum to output.
[shader("compute")]
[numthreads(1,1,1)]
void main(
    uniform RWStructuredBuffer<uint> inputA,
    uniform RWStructuredBuffer<uint> inputB,
    uniform RWStructuredBuffer<uint> output)
{
    uint a = inputA[0];
    uint b = inputB[0];
    output[0] = a + b;
}
