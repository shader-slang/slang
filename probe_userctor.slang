// Probe: a USER-defined constructor must KEEP its source-level debug info.
// (Regression guard for the #11550 fix, which suppresses debug info only for
// SYNTHESIZED initializers.)
struct Vec
{
    float x;
    float y;
    __init(float a, float b)
    {
        x = a;
        y = b;
    }
};

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Vec v = Vec(1.0, 2.0);
    outputBuffer[tid.x] = v.x + v.y;
}
