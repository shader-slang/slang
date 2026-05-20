// Verifies that function and branch coverage can be enabled without
// line coverage. The compiler should synthesize the shared coverage
// buffer and emit function-entry counters plus branch-arm counters
// for loop conditions, if/else and if-without-else arms, switch
// case/default arms, and the implicit no-match arm of a switch without
// default.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-function-coverage -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

uint helper(uint value)
{
    return value + 1u;
}

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
    if ((tid.x & 2u) != 0u)
    {
        accum += helper(tid.x);
    }
    uint whileIndex = 0;
    while (whileIndex < (tid.x & 3u))
    {
        accum += helper(whileIndex);
        ++whileIndex;
    }
    uint doIndex = 0;
    do
    {
        accum += doIndex;
        ++doIndex;
    } while (doIndex < ((tid.x & 1u) + 1u));
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

// Function + branch coverage for this shader emits exactly sixteen
// counters: two function-entry markers, two for-loop condition arms,
// two if/else arms, two if-without-else arms, two while-loop condition
// arms, two do-while condition arms, two switch case arms, one explicit
// switch default arm, and one implicit switch no-match/default arm.
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(3\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(4\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(5\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(6\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(7\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(8\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(9\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(10\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(11\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(12\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(13\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(14\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(15\)}}
//CHECK-NOT: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(16\)}}
