// Shader with an inlined helper function to test call-stack reconstruction.
uint add_values(uint x, uint y) { return x + y; }

[shader("compute")]
[numthreads(1,1,1)]
void main(
    uniform RWStructuredBuffer<uint> inputA,
    uniform RWStructuredBuffer<uint> inputB,
    uniform RWStructuredBuffer<uint> output)
{
    uint a = inputA[0];
    uint b = inputB[0];
    output[0] = add_values(a, b);
}
