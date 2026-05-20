// Verifies that function and branch coverage can be enabled without
// line coverage. The compiler should synthesize the shared coverage
// buffer and emit one function-entry counter plus branch-arm counters
// for the loop condition, nested if/else, switch case/default arms,
// and the implicit no-match arm of a switch without default.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = 0;
    for (uint i = 0; i < 4; ++i)
    {
        if ((i & 1u) == 0u)
            accum += i;
        else
            accum += i * 2u;
    }
    switch (int(tid.x) & 1)
    {
    case 0:
        accum += 10u;
        break;
    default:
        accum += 20u;
        break;
    }
    switch (int(tid.x) & 3)
    {
    case 2:
        accum += 30u;
        break;
    }
    outputBuffer[0] = accum;
}

// Shared synthesized coverage buffer should appear even though
// `-trace-coverage` line probes are not enabled.
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage

// Function + branch coverage for this shader emits exactly nine
// counters: one function-entry marker, two loop-condition arms,
// two if/else arms, two switch case arms, one explicit switch default arm,
// and one implicit switch no-match/default arm.
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(3\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(4\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(5\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(6\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(7\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(8\)}}
//CHECK-NOT: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(9\)}}
