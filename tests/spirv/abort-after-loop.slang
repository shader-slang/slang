//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort

// OpAbortKHR is a block terminator. When abort() is called from a loop body,
// SPIRV legalization must strip the abort block's original branch so the loop
// header and merge phis do not retain a predecessor that never branches.

RWStructuredBuffer<float> outBuf;

float loopValue(uint x)
{
    float v = 0.0;
    for (uint i = 0; i < 4; i++)
    {
        v += 1.0;
        if (i == x)
        {
            abort("loop: %u", i);
            v = 10.0;
        }
    }
    return v;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = loopValue(tid.x);
}

// CHECK: OpLoopMerge
// CHECK: OpAbortKHR
// CHECK-NEXT: %{{[a-zA-Z0-9_]+}} = OpLabel
