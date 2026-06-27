//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -O3

// OpAbortKHR is a block terminator. When abort() is called from one switch arm,
// SPIRV legalization must strip the abort block's original branch so the switch
// merge phi does not retain a predecessor that never branches.

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

// `switchValue` is not inlined, and the abort/case0/default/merge blocks are
// not guaranteed to be laid out in source order, so the facts below are
// checked independently (DAG) rather than via block order: the switch routes to
// all 3 blocks, the abort block contains OpAbortKHR, and -- the actual
// regression this test guards -- the merge phi has exactly the 2 non-aborting
// predecessors (no stray edge from the stripped abort branch).
// CHECK: OpSelectionMerge
// CHECK: OpSwitch {{.*}} [[DEFAULT:%[a-zA-Z0-9_]+]] 0 [[CASE0:%[a-zA-Z0-9_]+]] 1 [[ABORT:%[a-zA-Z0-9_]+]]
// CHECK-DAG: [[ABORT]] = OpLabel
// CHECK-DAG: OpAbortKHR
// CHECK-DAG: [[CASE0]] = OpLabel
// CHECK-DAG: [[DEFAULT]] = OpLabel
// CHECK-DAG: OpPhi %float %float_3 [[DEFAULT]] %float_1 [[CASE0]]{{$}}
