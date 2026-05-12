// Verifies that `-trace-coverage-reserved-space` reserves whole HLSL
// register spaces during coverage auto-allocation. Repeating the same
// reserved space is intentionally idempotent.

//TEST:SIMPLE(filecheck=CHECK_SPACE0):-target hlsl -stage compute -entry computeMain -profile cs_5_0 -trace-coverage -trace-coverage-reserved-space 0
//TEST:SIMPLE(filecheck=CHECK_DUP):-target hlsl -stage compute -entry computeMain -profile cs_5_0 -trace-coverage -trace-coverage-reserved-space 0 -trace-coverage-reserved-space 0

RWStructuredBuffer<uint> outputBuffer : register(u0, space0);

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        outputBuffer[0] = 1;
    }
}

//CHECK_SPACE0: _slang_coverage{{.*}}register(u0, space1)
//CHECK_DUP: _slang_coverage{{.*}}register(u0, space1)
