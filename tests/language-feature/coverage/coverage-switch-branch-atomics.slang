// Verifies branch coverage instrumentation for switch lowering edge cases:
// grouped case labels, fallthrough into a later body, and an explicit default.
// Counters are emitted at switch dispatch, so falling through from case 2 into
// default's body must not also count the default arm.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = 0;
    switch (int(tid.x) & 3)
    {
    case 0:
    case 1:
        accum += 10u;
        break;
    case 2:
        accum += 20u;
        // Intentional fallthrough. The default body executes, but the
        // default dispatch arm was not selected.
    default:
        accum += 30u;
        break;
    }
    outputBuffer[0] = accum;
}

// One counter is assigned to each selectable switch arm: case 0,
// case 1, case 2, and default.
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(3\)}}
//CHECK-NOT: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(4\)}}
