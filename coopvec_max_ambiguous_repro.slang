RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Create two CoopVec<int, 7> instances
    CoopVec<int, 7> vecA = CoopVec<int, 7>(1);
    CoopVec<int, 7> vecB = CoopVec<int, 7>(2);

    // This call causes ambiguous overload error
    // Slang incorrectly considers max<float, 7>(CoopVec<float, 7>, CoopVec<float, 7>)
    // as a candidate even though arguments are CoopVec<int, 7>
    CoopVec<int, 7> result = max(vecA, vecB);

    // Write result to avoid optimization
    outputBuffer[tid.x] = result[0];
}