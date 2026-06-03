// Verifies branch coverage for branch arms whose body exits the local
// control flow immediately. The counter must still be inserted at the
// start of the selected arm, before `continue`, `break`, or `return`
// transfers control away.

//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry computeMain -trace-branch-coverage
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain -trace-branch-coverage -profile cs_5_0

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
//CPP: RWStructuredBuffer{{.*}}_slang_coverage
//CPP-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(0\)}}
//CPP-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(1\)}}
//CPP-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(2\)}}
//CPP-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(3\)}}
//CPP-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(4\)}}
//CPP-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(5\)}}
//CPP-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(6\)}}
//CPP-DAG: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(7\)}}
//CPP-NOT: _slang_atomic_add_u64{{.*}}_slang_coverage{{.*int\(8\)}}

//HLSL: _slang_coverage
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(0\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(1\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(2\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(3\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(4\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(5\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(6\)}}
//HLSL-DAG: InterlockedAdd{{.*_slang_coverage.*int\(7\)}}
//HLSL-NOT: InterlockedAdd{{.*_slang_coverage.*int\(8\)}}
