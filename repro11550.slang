struct Pair
{
    float x;
    float y;
};

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p = {1.0, 2.0};
    outputBuffer[tid.x] = p.x + p.y;
}
