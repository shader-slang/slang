// Verifies the current branch coverage scope: statement-level control
// flow is instrumented, but expression-level short-circuit and ternary
// control flow are intentionally not branch-instrumented yet.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = tid.x;
    if ((tid.x != 0u) && (tid.y != 0u))
        value += 1u;
    else
        value += 2u;

    value += (tid.z != 0u) ? 3u : 4u;
    outputBuffer[0] = value;
}

// Only the statement-level if/else contributes branch counters.
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-NOT: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}
