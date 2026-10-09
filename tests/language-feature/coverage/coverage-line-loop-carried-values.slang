// Line-visit tracking on loop back edges must preserve loop-carried values
// and dominance-compatible block order through SPIR-V legalization.
//TEST:SIMPLE(filecheck=CHECK):-target spirv -profile spirv_1_5 -entry computeMain -trace-coverage -trace-branch-coverage -trace-coverage-counter-width 32 -validate-ir
//TEST:SIMPLE(filecheck=CHECK):-target spirv -profile spirv_1_5 -entry computeMain -trace-coverage -trace-branch-coverage -trace-coverage-counter-width 64 -validate-ir

StructuredBuffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float total = 0;
    float weight = 0;
    for (uint i = 0; i < tid.x; ++i)
    {
        for (uint j = 0; j < tid.y; ++j)
        {
            float value = inputBuffer[i + j];
            total += value;
            weight += value * value;
        }
    }
    outputBuffer[tid.x] = weight > 0 ? total / weight : 0;
}

//CHECK: OpAtomicIAdd
