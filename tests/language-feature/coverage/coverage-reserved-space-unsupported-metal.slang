// Verifies that `-trace-coverage-reserved-space` is explicit about
// target applicability. Metal has buffer indices but no descriptor
// space dimension, so reserved descriptor/register spaces do not
// affect auto-allocation.

//TEST:SIMPLE(filecheck=CHECK):-target metal -stage compute -entry computeMain -trace-coverage -trace-coverage-reserved-space 0

//CHECK: `-trace-coverage-reserved-space` does not apply to this target; ignoring reserved spaces

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        int x = 1 + 2;
    }
}
