// Verifies the branch-coverage-only lowering path for `if` without
// `else`. The pass must synthesize an empty false path so both the
// TrueArm and FalseArm are represented by coverage counters.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = tid.x;
    if ((tid.x & 1u) != 0u)
    {
        value += 1u;
    }
    outputBuffer[0] = value;
}

// No function or line coverage is enabled, so the synthetic else path
// should account for exactly the second branch-arm counter.
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-NOT: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}
