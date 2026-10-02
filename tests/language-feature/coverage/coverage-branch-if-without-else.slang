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

// Branch coverage records the decision line and its two outcomes,
// including false even though the source has no else statement.
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-NOT: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(3\)}}
