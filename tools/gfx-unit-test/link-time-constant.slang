extern static const bool turnOnFeature;
extern static const float constValue;

// Main entry-point. Write some value into buffer depending on link
// time constant.
[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint3 sv_dispatchThreadID: SV_DispatchThreadID,
    uniform RWStructuredBuffer<float> buffer)
{
    if (turnOnFeature)
    {
        buffer[0] = constValue;
    }
    else
    {
        buffer[0] = -1.0;
    }
}
