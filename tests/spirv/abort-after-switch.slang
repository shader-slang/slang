//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -skip-spirv-validation

// OpAbortKHR is a block terminator. When abort() is called from one switch arm,
// SPIRV legalization must strip the abort block's original branch so the switch
// merge phis do not retain a predecessor that never branches.

RWStructuredBuffer<float> outBuf;

float switchValue(uint x)
{
    float v = 0.0;
    switch (x & 3)
    {
    case 0:
        v = 1.0;
        break;
    case 1:
        abort("switch: %u", x);
        v = 2.0;
        break;
    default:
        v = 3.0;
        break;
    }
    return v;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = switchValue(tid.x);
}

// CHECK: OpSelectionMerge
// CHECK: OpSwitch
// CHECK: OpAbortKHR
// CHECK-NEXT: %{{[a-zA-Z0-9_]+}} = OpLabel
