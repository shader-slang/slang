// Verifies branch coverage for branch arms whose body exits the local
// control flow immediately. The counter must still be inserted at the
// start of the selected arm, before `continue`, `break`, or `return`
// transfers control away.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-branch-coverage

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint value = tid.x;
    for (uint i = 0; i < 4; ++i)
    {
        if ((i & 1u) == 0u)
        {
            continue;
        }
        else
        {
            value += i;
        }

        if (i == tid.x)
        {
            break;
        }
        else
        {
            value += 10u;
        }
    }

    if ((tid.x & 1u) != 0u)
    {
        outputBuffer[0] = value;
        return;
    }
    else
    {
        value += 100u;
    }

    outputBuffer[0] = value;
}

// Expected counters:
//   - loop condition true/false
//   - continue if true/false
//   - break if true/false
//   - return if true/false
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(0\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(1\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(2\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(3\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(4\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(5\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(6\)}}
//CHECK-DAG: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(7\)}}
//CHECK-NOT: _slang_atomic_add_u32{{.*}}_slang_coverage{{.*int\(8\)}}
